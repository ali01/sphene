#include "arp_packet.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>
#include "ethernet_packet.h"
#include "fwk/buffer.h"
#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"


const size_t
ARPPacket::kHeaderSize = sizeof(struct ARPHeader);

ARPPacket::ARPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      arp_hdr_((struct ARPHeader *)offsetAddress(0)) {
  // NOTE(ms): No validation of the fields is done here for performance
  //   reasons. We want to be able to create ARP packets inside of
  //   pre-allocated buffers, so to avoid the chicken-egg problem, we require
  //   validation to be done by the users of this interface.
}


void ARPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


ARPPacket::Operation ARPPacket::operation() const {
  return (Operation)(ntohs(arp_hdr_->oper));
}


void ARPPacket::operationIs(const Operation& op) {
  arp_hdr_->oper = htons(op);
}


// TODO(ms): This needs tests.
std::string ARPPacket::operationName() const {
  switch (operation()) {
    case kRequest:
      return "request";
      break;
    case kReply:
      return "reply";
      break;
    default:
      return "unknown";
      break;
  }
}


EthernetAddr ARPPacket::senderHWAddr() const {
  return EthernetAddr(arp_hdr_->sha);
}


void ARPPacket::senderHWAddrIs(const EthernetAddr& addr) {
  memcpy(arp_hdr_->sha, addr.data(), EthernetAddr::kAddrLen);
}


EthernetAddr ARPPacket::targetHWAddr() const {
  return EthernetAddr(arp_hdr_->tha);
}


void ARPPacket::targetHWAddrIs(const EthernetAddr& addr) {
  memcpy(arp_hdr_->tha, addr.data(), EthernetAddr::kAddrLen);
}


IPv4Addr ARPPacket::senderPAddr() const {
  return ntohl(arp_hdr_->spa);
}


void ARPPacket::senderPAddrIs(const IPv4Addr& addr) {
  arp_hdr_->spa = addr.nbo();
}


IPv4Addr ARPPacket::targetPAddr() const {
  return ntohl(arp_hdr_->tpa);
}


void ARPPacket::targetPAddrIs(const IPv4Addr& addr) {
  arp_hdr_->tpa = addr.nbo();
}
