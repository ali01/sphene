#include "icmp_packet.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>
#include "fwk/buffer.h"
#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"


ICMPPacket::ICMPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      icmp_hdr_((struct ICMPHeader *)offsetAddress(0)) { }


void ICMPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


ICMPPacket::Type ICMPPacket::type() const {
  return (Type)(icmp_hdr_->type);
}


void ICMPPacket::typeIs(const Type t) {
  uint8_t tn = t;
  icmp_hdr_->type = tn;
}


// TODO(ms): This needs tests.
std::string typeName() const {
  switch (type()) {
    case kEchoReply:
      return "echo reply";
      break;
    case kDestUnreachable:
      return "destination unreachable";
      break;
    case kEchoRequest:
      return "echo request";
      break;
    case kTimeExceeded:
      return "time exceeded";
      break;
    default:
      return "unknown";
      break;
  }
}


uint8_t ICMPPacket::code() const {
  return icmp_hdr_->code;
}


void ICMPPacket::codeIs(const uint8_t code) {
  icmp_hdr_->code = code;
}


uint16_t ICMPPacket::checksum() const {
  return ntohs(icmp_hdr_->cksum);
}


void ICMPPacket::checksumIs(const uint16_t ck) {
  icmp_hdr_->cksum = htons(ck);
}


void ICMPPacket::checksumReset() {
  checksumIs(0);
  checksumIs(computeChecksum());
}


// TODO(ms): This should be moved to a util.h/util.cc pair.
// TODO(ms): This is not correct. Checksums in ICMP are calculated over the
//   header and data, which is not yet part of the above implementation.
uint16_t ICMPPacket::computeChecksum() const {
  uint32_t sum;
  unsigned int length = len();
  const uint8_t *header = (const uint8_t *)icmp_hdr_;

  for (sum = 0; length >= 2; header += 2, length -= 2)
    sum += header[0] << 8 | header[1];

  if (len > 0)
    sum += header[0] << 8;

  while (sum > 0xFFFF)
    sum = (sum >> 16) + (sum & 0xFFFF);

  sum = ~sum;

  if (sum == 0)
    sum = 0xFFFF;

  return sum;
}
