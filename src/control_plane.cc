#include "control_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "ip_packet.h"


ControlPlane::ControlPlane(const std::string& name)
    : Fwk::NamedInterface(name),
      log_(Fwk::Log::LogNew(name)),
      functor_(this) { }


void ControlPlane::packetNew(EthernetPacket::Ptr pkt,
                             const Interface::PtrConst iface) {
  // Dispatch packet using double-dispatch.
  (*pkt)(&functor_, iface);
}


ControlPlane::PacketFunctor::PacketFunctor(ControlPlane* const cp)
    : cp_(cp), log_(cp->log_) { }


void ControlPlane::PacketFunctor::operator()(ARPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  // TODO(ms): Validate all of the fields in the ARP packet.

  DLOG << "ARPPacket dispatch in ControlPlane";
  DLOG << "  operation: " << pkt->operationName();
  DLOG << "  sender HW addr: " << (string)pkt->senderHWAddr();
  DLOG << "  sender IP addr: " << (string)pkt->senderPAddr();
  DLOG << "  target HW addr: " << (string)pkt->targetHWAddr();
  DLOG << "  target IP addr: " << (string)pkt->targetPAddr();

  IPv4Addr sender_ip = pkt->senderPAddr();
  IPv4Addr target_ip = pkt->targetPAddr();
  EthernetAddr sender_eth = pkt->senderHWAddr();
  EthernetAddr target_eth = pkt->targetHWAddr();
  bool merge_flag = false;

  ARPCache::Entry::Ptr cache_entry = cp_->ethernetCache()->entry(sender_ip);
  if (cache_entry) {
    cache_entry->ethernetAddrIs(sender_eth);
    cache_entry->ageIs(0);
    merge_flag = true;
  }

  // Are we the target of the ARP packet?
  if (cp_->dataPlane()->interfaceMap()->interfaceAddr(target_ip)) {
    DLOG << "We are the target of this ARP packet.";

    if (!merge_flag) {
      cache_entry = ARPCache::Entry::EntryNew(sender_ip, sender_eth);
      cp_->ethernetCache()->entryIs(sender_ip, cache_entry);
    }

    // Look at the opcode.
    if (pkt->operation() == ARPPacket::kRequest) {
      DLOG << "sending ARP reply";

      // Swap the hardware and protocol fields.
      pkt->targetHWAddrIs(sender_eth);
      pkt->targetPAddrIs(sender_ip);
      pkt->senderHWAddrIs(target_eth);
      pkt->senderPAddrIs(target_ip);

      // Flip the operation.
      pkt->operationIs(ARPPacket::kReply);

      // Get the enclosing EthernetPacket.
      EthernetPacket::Ptr eth_pkt =
          (EthernetPacket*)(pkt->enclosingPacket().ptr());

      // Update the source address to our MAC.
      eth_pkt->srcIs(iface->mac());

      // Send ARP reply on same interface.
      cp_->dataPlane()->outputPacketNew(eth_pkt, iface);

    } else if (pkt->operation() == ARPPacket::kReply) {
      DLOG << "ARP reply handling unimplemented";
    }
  }
}


void ControlPlane::PacketFunctor::operator()(EthernetPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "EthernetPacket dispatch in ControlPlane";

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void ControlPlane::PacketFunctor::operator()(ICMPPacket* const pkt,
                                             const Interface::PtrConst iface) {

}


void ControlPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in ControlPlane";
}


void ControlPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                             const Interface::PtrConst iface) {

}
