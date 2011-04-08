#include "arp_packet.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include "ethernet_packet.h"
#include "interface.h"


ARPPacket::ARPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      arp_hdr_((struct ARPHeader *)offsetAddress(0)) {
  // TODO(ms): error checking here (htype, ptype, ...).
}


void ARPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


ARPPacket::Operation ARPPacket::operation() const {
  switch (ntohs(arp_hdr_->oper)) {
    case kRequest:
      return kRequest;
      break;
    case kReply:
    default:
      return kReply;
      break;
  }
}


void ARPPacket::operationIs(const Operation& op) {
  arp_hdr_->oper = htons(op);
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
  return IPv4Addr(arp_hdr_->spa);
}


void ARPPacket::senderPAddrIs(const IPv4Addr& addr) {
  arp_hdr_->spa = htonl((uint32_t)addr);
}


IPv4Addr ARPPacket::targetPAddr() const {
  return IPv4Addr(arp_hdr_->tpa);
}


void ARPPacket::targetPAddrIs(const IPv4Addr& addr) {
  arp_hdr_->tpa = htonl((uint32_t)addr);
}
