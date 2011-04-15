#include "data_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "control_plane.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "ip_packet.h"


DataPlane::DataPlane(const std::string& name,
                     struct sr_instance *sr,
                     RoutingTable::Ptr routing_table,
                     ARPCache::Ptr arp_cache)
    : Fwk::NamedInterface(name),
      log_(Fwk::Log::LogNew(name)),
      functor_(this),
      iface_map_(InterfaceMap::InterfaceMapNew()),
      routing_table_(routing_table),
      arp_cache_(arp_cache),
      cp_(NULL),
      sr_(sr) { }


void DataPlane::packetNew(Packet::Ptr pkt,
                          const Interface::PtrConst iface) {
  // Dispatch packet using double-dispatch.
  (*pkt)(&functor_, iface);
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
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();
  DLOG << "  type: " << pkt->typeName();

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void DataPlane::PacketFunctor::operator()(ICMPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "ICMPPacket dispatch in DataPlane";
  DLOG << "  type: " << pkt->type() << " (" << pkt->typeName() << ")";
  DLOG << "  code: " << (uint32_t)pkt->code();
}


void DataPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in DataPlane";
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();

  // Packet validation.
  if (pkt->buffer()->len() < 5) {
    DLOG << "  packet buffer too small: " << (uint32_t)pkt->buffer()->len();
    return;
  }
  if (pkt->headerLength() < 5) {  // in words, not bytes
    DLOG << "  header length too small: " << (uint32_t)pkt->headerLength();
    return;
  }
  if (pkt->version() != 4) {
    DLOG << "  invalid IP version: " << (uint32_t)pkt->version();
    return;
  }
  if (!pkt->checksumValid()) {
    DLOG << "  invalid checksum";
    return;
  }

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
    RoutingTable::ScopedLock lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "Routing table entry not found";
    // Send to control plane for error processing.
    dp_->controlPlane()->outputPacketNew(pkt, iface);
    return;
  }

  // Outgoing interface.
  Interface::Ptr out_iface = r_entry->interface();

  DLOG << "  LPM for " << dest_ip << ": " << r_entry->subnet()
       << " (" << out_iface->name() << ")";

  // Next hop IP address.
  IPv4Addr next_hop_ip = r_entry->gateway();

  // Look up ARP entry for next hop.
  ARPCache::Entry::Ptr arp_entry =
      dp_->controlPlane()->arpCache()->entry(next_hop_ip);
  if (!arp_entry) {
    // ARP cache miss. Send packet to control plane to be forwarded.
    dp_->controlPlane()->outputPacketNew(pkt, iface);
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
    dp_->controlPlane()->outputPacketNew(pkt, iface);
    return;
  }

  // Recompute the checksum since we changed the TTL.
  pkt->checksumReset();

  // Update Ethernet header using ARP entry.
  EthernetPacket::Ptr eth_pkt =
      Ptr::st_cast<EthernetPacket>(pkt->enclosingPacket());
  eth_pkt->srcIs(out_iface->mac());
  eth_pkt->dstIs(arp_entry->ethernetAddr());

  // Send packet.
  DLOG << "Forwarding IP packet to " << string(next_hop_ip);
  dp_->outputPacketNew(eth_pkt, out_iface);
}


void DataPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "UnknownPacket dispatch in DataPlane";
}
