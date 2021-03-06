#include "arp_packet.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "fwk/exception.h"
#include "fwk/log.h"

#include "ethernet_packet.h"
#include "interface.h"
#include "ip_packet.h"
#include "ipv4_addr.h"
#include "packet_buffer.h"


/* Static global log instance */
static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("ARPPacket");

const size_t
ARPPacket::kHeaderSize = sizeof(struct ARPHeader);

ARPPacket::ARPPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset)
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


bool
ARPPacket::valid() const {
  // Verify length.
  if (len() != kPacketLen) {
    DLOG << "Packet length is too short";
    DLOG << "  Expected: " << kPacketLen;
    DLOG << "  Actual:   " << len();
    return false;
  }

  // Verify hardware and protocol types and lengths.
  if (hwType() != kEthernet) {
    DLOG << "Packet hardware type is not ethernet.";
    return false;
  }

  if (hwAddrLen() != EthernetAddr::kAddrLen) {
    DLOG << "Packet hardware address length is not that of ethernet.";
    return false;
  }

  if (pType() != kIP) {
    DLOG << "Packet protocol address is not IP.";
    return false;
  }

  if (pAddrLen() != IPv4Addr::kAddrLen) {
    DLOG << "Packet protocol address length is not that of IP";
    return false;
  }

  // Verify operation.
  if (operation() != kRequest && operation() != kReply) {
    DLOG << "Packet operation field is incorrect.";
    return false;
  }

  return true;
}


ARPPacket::HWType ARPPacket::hwType() const {
  return (HWType)(ntohs(arp_hdr_->htype));
}


void ARPPacket::hwTypeIs(HWType hwtype) {
  arp_hdr_->htype = htons(hwtype);

  if (hwtype == kEthernet)
    arp_hdr_->hlen = EthernetAddr::kAddrLen;
  else
    arp_hdr_->hlen = 0;
}


ARPPacket::PType ARPPacket::pType() const {
  return (PType)(ntohs(arp_hdr_->ptype));
}


void ARPPacket::pTypeIs(PType ptype) {
  arp_hdr_->ptype = htons(ptype);

  if (ptype == kIP)
    arp_hdr_->plen = IPv4Addr::kAddrLen;
  else
    arp_hdr_->plen = 0;
}


uint8_t ARPPacket::hwAddrLen() const {
  return arp_hdr_->hlen;
}


uint8_t ARPPacket::pAddrLen() const {
  return arp_hdr_->plen;
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
