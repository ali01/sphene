#include "icmp_packet.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>
#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"
#include "packet_buffer.h"
#include "unknown_packet.h"

const size_t ICMPPacket::kHeaderLen;


ICMPPacket::ICMPPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      icmp_hdr_((struct ICMPHeader *)offsetAddress(0)) { }


void ICMPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


bool ICMPPacket::valid() const {
  // Verify length.
  if (len() < kHeaderLen)
    return false;

  // Verify checksum.
  if (!checksumValid())
    return false;

  return true;
}


ICMPPacket::Type ICMPPacket::type() const {
  return (Type)(icmp_hdr_->type);
}


void ICMPPacket::typeIs(const Type t) {
  uint8_t tn = t;
  icmp_hdr_->type = tn;
}


// TODO(ms): This needs tests.
std::string ICMPPacket::typeName() const {
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


ICMPPacket::Code ICMPPacket::code() const {
  return (Code)icmp_hdr_->code;
}


void ICMPPacket::codeIs(const Code code) {
  icmp_hdr_->code = code;
}


uint16_t ICMPPacket::checksum() const {
  return ntohs(icmp_hdr_->cksum);
}


void ICMPPacket::checksumIs(const uint16_t ck) {
  icmp_hdr_->cksum = htons(ck);
}


bool ICMPPacket::checksumValid() const {
  // Checksum field is present.
  uint16_t pkt_cksum = checksum();

  ICMPPacket* pkt = const_cast<ICMPPacket*>(this);
  uint16_t calculated_cksum = pkt->checksumReset();
  pkt->checksumIs(pkt_cksum);
  return (pkt_cksum == calculated_cksum);
}


uint16_t ICMPPacket::checksumReset() {
  checksumIs(0);
  const uint16_t ck = computeChecksum();
  checksumIs(ck);
  return ck;
}


// TODO(ms): This should be moved to a util.h/util.cc pair.
// TODO(ms): This is not correct. Checksums in ICMP are calculated over the
//   header and data, which is not yet part of the above implementation.
// NOTE(ali): IPPacket now exposes a public static compute_checksum() function
//   that can be used to implement this. After all, this is widely known to be
//   "The IP Checksum." See the implementation of the OSPFPackets checksum
//   functions.
uint16_t ICMPPacket::computeChecksum() const {
  uint32_t sum;
  unsigned int length = len();
  const uint8_t *header = (const uint8_t *)icmp_hdr_;

  for (sum = 0; length >= 2; header += 2, length -= 2)
    sum += header[0] << 8 | header[1];

  if (length > 0)
    sum += header[0] << 8;

  while (sum > 0xFFFF)
    sum = (sum >> 16) + (sum & 0xFFFF);

  sum = ~sum;

  if (sum == 0)
    sum = 0xFFFF;

  return sum;
}


ICMPTimeExceededPacket::ICMPTimeExceededPacket(PacketBuffer::Ptr buffer,
                                               unsigned int buffer_offset)
    : ICMPPacket(buffer, buffer_offset) {
  typeIs(kTimeExceeded);
  codeIs(kTTLExceeded);
  icmp_hdr_->rest = 0;
}


// TODO(ms): Move common code to a protected init() method.
ICMPTimeExceededPacket::ICMPTimeExceededPacket(ICMPPacket::Ptr icmp_pkt)
    : ICMPPacket(icmp_pkt->buffer(), icmp_pkt->bufferOffset()) {
  typeIs(kTimeExceeded);
  codeIs(kTTLExceeded);
  icmp_hdr_->rest = 0;
}


void ICMPTimeExceededPacket::operator()(Functor* const f,
                                        const Interface::PtrConst iface) {
  (*f)(this, iface);
}


void ICMPTimeExceededPacket::originalPacketIs(IPPacket::PtrConst pkt) {
  uint8_t* const ip_data = offsetAddress(kHeaderLen);

  // Copy at most IP header length + 8 bytes.
  const size_t max_len = pkt->headerLen() + 8;
  const size_t len = (pkt->len() <= max_len) ? pkt->len() : max_len;

  // Copy data.
  memcpy(ip_data, pkt->data(), len);
}


ICMPDestUnreachablePacket::ICMPDestUnreachablePacket(PacketBuffer::Ptr buffer,
                                                     unsigned int buffer_offset)
    : ICMPPacket(buffer, buffer_offset) {
  typeIs(kDestUnreachable);
  codeIs(kHostUnreach);
  icmp_hdr_->rest = 0;
}


// TODO(ms): Move common code to a protected init() method.
ICMPDestUnreachablePacket::ICMPDestUnreachablePacket(ICMPPacket::Ptr icmp_pkt)
    : ICMPPacket(icmp_pkt->buffer(), icmp_pkt->bufferOffset()) {
  typeIs(kDestUnreachable);
  codeIs(kHostUnreach);
  icmp_hdr_->rest = 0;
}


void ICMPDestUnreachablePacket::operator()(Functor* const f,
                                           const Interface::PtrConst iface) {
  (*f)(this, iface);
}


void ICMPDestUnreachablePacket::originalPacketIs(IPPacket::PtrConst pkt) {
  uint8_t* const ip_data = offsetAddress(kHeaderLen);

  // Copy at most IP header length + 8 bytes.
  const size_t max_len = pkt->headerLen() + 8;
  const size_t len = (pkt->len() <= max_len) ? pkt->len() : max_len;

  // Copy data.
  memcpy(ip_data, pkt->data(), len);
}
