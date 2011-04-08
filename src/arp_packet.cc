#include "arp_packet.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include "ethernet_packet.h"
#include "fwk/buffer.h"
#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"


ARPPacket::ARPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      arp_hdr_((struct ARPHeader *)offsetAddress(0)) {
  // Validate length.
  if (buffer->size() - buffer_offset != kPacketLen) {
    throw Fwk::RangeException("ARPPacket::ARPPacket",
                              "invalid packet size");
  }

  // Validate types.
  if (ntohs(arp_hdr_->htype) != 0x0001)  // 0x0001 = Ethernet
    throw Fwk::RangeException("ARPPacket::ARPPacket",
                              "unexpected value in htype field");
  if (ntohs(arp_hdr_->ptype) != EthernetPacket::kIP)
    throw Fwk::RangeException("ARPPacket::ARPPacket",
                              "unexpected value in ptype field");

  // Validate address lengths.
  if (arp_hdr_->hlen != EthernetAddr::kAddrLen)
    throw Fwk::RangeException("ARPPacket::ARPPacket",
                              "unexpected value in hlen field");
  if (arp_hdr_->plen != IPv4Addr::kAddrLen)
    throw Fwk::RangeException("ARPPacket::ARPPacket",
                              "unexpected value in plen field");

  // Validate operation field.
  if (ntohs(arp_hdr_->oper) != kRequest && arp_hdr_->oper != kReply)
    throw Fwk::RangeException("ARPPacket::ARPPacket",
                              "unexpected value in operation field");
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
