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

  // TODO(ms): Look in ARP cache. If the sender IP address is already in the
  //   ARP cache, then update the sender hardware address field of the entry in
  //   the ARP cache and set the merge_flag.
  //...

  // Are we the target of the ARP packet?
  if (cp_->dataPlane()->interfaceMap()->interfaceAddr(pkt->targetPAddr())) {
    DLOG << "we are the target of this ARP packet";

    // TODO(ms): Add the <sender IP, sender HW> to the ARP cache if it wasn't
    //   there already.
    //...

    // Look at the opcode.
    if (pkt->operation() == ARPPacket::kRequest) {
      DLOG << "sending ARP reply";

      // Swap the hardware and protocol fields.
      EthernetAddr our_hwaddr = pkt->targetHWAddr();
      IPv4Addr our_ipaddr = pkt->targetPAddr();
      pkt->targetHWAddrIs(pkt->senderHWAddr());
      pkt->targetPAddrIs(pkt->senderPAddr());
      pkt->senderHWAddrIs(our_hwaddr);
      pkt->senderPAddrIs(our_ipaddr);

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
