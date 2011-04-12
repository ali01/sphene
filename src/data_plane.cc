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

  if (pkt->buffer()->len() >= 5 && pkt->headerLength() >= 5 &&
      pkt->version() == 4 && pkt->checksumValid()) {

    // IP Packet's destination
    IPv4Addr dest_ip = pkt->dst();
    Interface::Ptr iface = dp_->iface_map_->interface(dest_ip);

    if (iface == NULL) { // Packet is not destined to router
      // Decrementing TTL
      pkt->ttlDec(1);

      if (pkt->ttl() > 0) {
        // Recomputing checksum
        pkt->checksumReset();

        // Finding longest prefix match
        RoutingTable::Entry::Ptr r_entry = dp_->routing_table_->lpm(dest_ip);
        if (r_entry) {
          IPv4Addr next_hop_ip = r_entry->gateway();
          ARPCache::Entry::Ptr arp_entry = dp_->arp_cache_->entry(next_hop_ip);

          if (arp_entry) {
            Interface::Ptr out_iface = r_entry->interface();
            EthernetPacket::Ptr eth_pkt =
              Ptr::st_cast<EthernetPacket>(pkt->enclosingPacket());

            eth_pkt->srcIs(out_iface->mac());
            eth_pkt->dstIs(arp_entry->ethernetAddr());

            // Forwarding packet.
            dp_->outputPacketNew(eth_pkt, out_iface);

          } else {
            // ARP cache miss; dispatch to control plane.
            dp_->controlPlane()->packetNew(pkt, iface);
          }

        } else {
          DLOG << "Route for " << string(dest_ip)
               << " does not exist in RoutingTable.";
          // TODO: send ICMP no route to host.
        }

      } else {
        // Send ICMP Time Exceeded Message to source
        // TODO
      }

    } else {
      // Packet is destined to router; dispatch payload to control plane.
      dp_->controlPlane()->packetNew(pkt, iface);
    }
  }
}


void DataPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "UnknownPacket dispatch in DataPlane";
}
