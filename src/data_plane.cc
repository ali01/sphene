#include "data_plane.h"

#include <string>

#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/scoped_lock.h"

#include "arp_packet.h"
#include "control_plane.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "ip_packet.h"
#include "sr_integration.h"


DataPlane::DataPlane(const std::string& name,
                     struct sr_instance *sr,
                     RoutingTable::Ptr routing_table,
                     ARPCache::Ptr arp_cache)
    : Fwk::NamedInterface(name),
      log_(Fwk::Log::LogNew(name)),
      iface_map_(InterfaceMap::InterfaceMapNew()),
      routing_table_(routing_table),
      arp_cache_(arp_cache),
      cp_(NULL),
      sr_(sr),
      functor_(this) { }


void DataPlane::packetNew(Packet::Ptr pkt,
                          const Interface::PtrConst iface) {
  // Ignore packets on a disabled interface.
  if (!iface->enabled()) {
    DLOG << "Ignoring packet on disabled interface " << iface->name();
    return;
  }

  // Dispatch packet using double-dispatch.
  (*pkt)(&functor_, iface);
}


void DataPlane::outputPacketNew(EthernetPacket::PtrConst pkt,
                                Interface::PtrConst iface) {
  DLOG << "outputPacketNew() in DataPlane";
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();
  DLOG << "  length: " << pkt->len();
  DLOG << "  type: " << pkt->typeName();

  if (pkt->type() == EthernetPacket::kIP) {
    EthernetPacket* _pkt = const_cast<EthernetPacket*>(pkt.ptr());
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(_pkt->payload());
    DLOG << "    src: " << ip_pkt->src();
    DLOG << "    dst: " << ip_pkt->dst();
    DLOG << "    identification: " << ip_pkt->identification();
    DLOG << "    flags: " << (uint32_t)ip_pkt->flags();
    DLOG << "    fragment offset: " << ip_pkt->fragmentOffset();
    DLOG << "    ttl: " << (uint32_t)ip_pkt->ttl();
    DLOG << "    protocol: " << ip_pkt->protocol();
    DLOG << "    header checksum: " << ip_pkt->checksum();
    DLOG << "    length: " << ip_pkt->packetLength();
  }

  struct sr_instance* sr = instance();
  sr_integ_low_level_output(sr, pkt->data(), pkt->len(), iface->name().c_str());
}


InterfaceMap::Ptr DataPlane::interfaceMap() const {
  return iface_map_;
}


DataPlane::PacketFunctor::PacketFunctor(DataPlane* const dp)
    : dp_(dp), log_(dp->log_) { }


void DataPlane::PacketFunctor::operator()(ARPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "ARPPacket dispatch in DataPlane";

  // Dispatch ARP packets to control plane.
  EthernetPacket::Ptr eth_pkt = (EthernetPacket*)(pkt->enclosingPacket().ptr());
  dp_->controlPlane()->packetNew(eth_pkt, iface);
}


void DataPlane::PacketFunctor::operator()(EthernetPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "EthernetPacket dispatch in DataPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();
  DLOG << "  type: " << pkt->typeName();

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void DataPlane::PacketFunctor::operator()(GREPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "GREPacket dispatch in DataPlane";
}


void DataPlane::PacketFunctor::operator()(ICMPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "ICMPPacket dispatch in DataPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  DLOG << "  type: " << pkt->type() << " (" << pkt->typeName() << ")";
  DLOG << "  code: " << (uint32_t)pkt->code();
}


void DataPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in DataPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();

  // Look up IP Packet's destination.
  IPv4Addr dest_ip = pkt->dst();
  Interface::Ptr target_iface = dp_->interfaceMap()->interfaceAddr(dest_ip);

  // IP packets destined for the router go to the control plane immediately.
  if (target_iface) {
    dp_->controlPlane()->packetNew(pkt, iface);
    return;
  }

  // Otherwise, we need to forward the packet.

  // Look up routing table entry of the longest prefix match.
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    Fwk::ScopedLock<RoutingTable>lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "No route to " << dest_ip;
    // Send to control plane for error processing.
    dp_->controlPlane()->outputPacketNew(pkt);
    return;
  }

  // Outgoing interface.
  Interface::PtrConst out_iface = r_entry->interface();
  if (!out_iface->enabled()) {
    // We have a route, but can't reach the target network. The control plane
    // will deal with this case by sending an ICMP message.
    dp_->controlPlane()->outputPacketNew(pkt);
    return;
  }
  if (r_entry->type() == RoutingTable::Entry::kStatic &&
      out_iface->type() == Interface::kVirtual) {
    // Let ControlPlane deal with sending out virtual interfaces.
    dp_->controlPlane()->outputPacketNew(pkt);
    return;
  }

  DLOG << "  LPM for " << dest_ip << ": " << r_entry->subnet()
       << " (" << out_iface->name() << ")";

  // Next hop IP address.
  IPv4Addr next_hop_ip = r_entry->gateway();
  if (next_hop_ip == 0)
    next_hop_ip = pkt->dst();

  // Look up ARP entry for next hop.
  ARPCache::Entry::Ptr arp_entry;
  {
    ARPCache::Ptr cache = dp_->controlPlane()->arpCache();
    Fwk::ScopedLock<ARPCache>lock(cache);
    arp_entry = cache->entry(next_hop_ip);
  }
  if (!arp_entry) {
    // ARP cache miss. Send packet to control plane to be forwarded.
    dp_->controlPlane()->outputPacketNew(pkt);
    return;
  }

  // Decrement TTL. We do this last to avoid having to increase the TTL and
  // recompute the checksum again if an error occurs.
  pkt->ttlDec(1);
  DLOG << "  decremented TTL: " << (uint32_t)pkt->ttl();
  if (pkt->ttl() < 1) {
    // Send ICMP Time Exceeded Message to source.
    pkt->ttlIs(pkt->ttl() + 1);
    // Send to control plane for error processing.
    dp_->controlPlane()->outputPacketNew(pkt);
    return;
  }

  // Recompute the checksum since we changed the TTL.
  pkt->checksumReset();

  // Add Ethernet header using ARP entry data.
  pkt->buffer()->minimumSizeIs(pkt->len() + EthernetPacket::kHeaderSize);
  EthernetPacket::Ptr eth_pkt =
      EthernetPacket::New(pkt->buffer(),
                          pkt->bufferOffset() - EthernetPacket::kHeaderSize);
  eth_pkt->srcIs(out_iface->mac());
  eth_pkt->dstIs(arp_entry->ethernetAddr());
  eth_pkt->typeIs(EthernetPacket::kIP);


  // Send packet.
  DLOG << "Forwarding IP packet to " << string(next_hop_ip);
  dp_->outputPacketNew(eth_pkt, out_iface);
}


void DataPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "UnknownPacket dispatch in DataPlane";
}
