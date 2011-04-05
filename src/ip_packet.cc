#include "ip_packet.h"

#include <netinet/in.h>

IPPacket::IPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      ip_hdr_((struct ip_hdr *)offsetAddress(0)) {}

IPVersion
IPPacket::version() const {
  return ip_hdr_->ip_v;
}

void
IPPacket::versionIs(const IPVersion& version) {
  ip_hdr_->ip_v = version;
}

Protocol
IPPacket::protocol() const {
  return ip_hdr_->ip_p;
}

void
IPPacket::protocolIs(const Protocol& protocol) {
  ip_hdr_->ip_p = protocol;
}

uint8_t
IPPacket::ttl() const {
  return ip_hdr_->ip_ttl;
}

void
IPPacket::ttlIs(uint8_t ttl) {
  ip_hdr_->ip_ttl = ttl;
}

void
IPPacket::ttlDec(uint8_t dec_amount) {
  ip_hdr_->ip_ttl -= dec_amount;
}

IPv4Addr
IPPacket::src() const {
  return ntohl(ip_hdr_->ip_src);
}

void
IPPacket::srcIs(const IPv4Addr& src) {
  ip_hdr_->ip_src = htonl(src);
}

IPv4Addr
IPPacket::dst() const {
  return ntohl(ip_hdr_->ip_dst);
}

void
IPPacket::dstIs(const IPv4Addr& dst) {
  ip_hdr_->ip_dst = htonl(dst);
}

uint16_t
IPPacket::checksum() const {
  return ntohs(ip_hdr_->ip_sum);
}

void
IPPacket::checksumIs(uint16_t ck) {
  ip_hdr_->ip_sum = htons(ck);
}

void
IPPacket::checksumReset() {
  checksumIs(0);
  checksumIs(compute_cksum());
}

uint16_t
IPPacket::compute_cksum() const {
  uint32_t sum;
  unsigned int len = sizeof(struct ip_hdr);
  const uint8_t *header = (const uint8_t *)ip_hdr_;

  for (sum = 0; len >= 2; header += 2, len -= 2)
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
