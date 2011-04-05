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
