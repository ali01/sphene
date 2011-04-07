#include "ip_packet.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include "interface.h"

/* IP Header packet struct. */

struct ip_hdr {
  uint8_t ip_v_hl;          /* version and header length */
  uint8_t ip_tos;           /* type of service */
  uint16_t ip_len;          /* total length */
  uint16_t ip_id;           /* identification */
  uint16_t ip_fl_off;       /* flags and fragment offset */
  uint8_t ip_ttl;           /* time to live */
  uint8_t ip_p;             /* protocol */
  uint16_t ip_sum;          /* checksum */
  uint32_t ip_src;          /* source address */
  uint32_t ip_dst;          /* destination address */
} __attribute__((packed));


/* IPv4Addr */

IPv4Addr::IPv4Addr(const std::string& addr) {
  /* TODO(ms): throw exception on failure? */
  if (!inet_pton(AF_INET, addr.c_str(), &addr_))
    addr_ = 0;
}

IPv4Addr::IPv4Addr(const char* const addr) {
  if (!inet_pton(AF_INET, addr, &addr_))
    addr_ = 0;
}

bool
IPv4Addr::operator==(uint32_t other) const {
  return addr_ == htonl(other);
}

IPv4Addr::operator uint32_t() const {
  return ntohl(addr_);
}

IPv4Addr::operator std::string() const {
  char buf[INET_ADDRSTRLEN + 1];
  inet_ntop(AF_INET, (struct in_addr*)&addr_, buf, sizeof(buf));
  buf[INET_ADDRSTRLEN] = 0;
  return buf;
}


/* IPPacket */


IPPacket::IPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      ip_hdr_((struct ip_hdr *)offsetAddress(0)) {}


void IPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


IPVersion
IPPacket::version() const {
  return ip_hdr_->ip_v_hl >> 4;
}

void
IPPacket::versionIs(const IPVersion& version) {
  ip_hdr_->ip_v_hl = (version << 4) | (ip_hdr_->ip_v_hl & 0x0F);
}

IPHeaderLength
IPPacket::headerLength() const {
  return ip_hdr_->ip_v_hl & 0x0F;
}

void
IPPacket::headerLengthIs(const IPHeaderLength& len) {
  ip_hdr_->ip_v_hl = (len & 0x0F) | (ip_hdr_->ip_v_hl & 0xF0);
}

uint16_t
IPPacket::packetLength() const {
  return ntohs(ip_hdr_->ip_len);
}

void
IPPacket::packetLengthIs(uint16_t len) {
  ip_hdr_->ip_len = htons(len);
}

IPDiffServices
IPPacket::diffServices() const {
  return ip_hdr_->ip_tos;
}

void
IPPacket::diffServicesAre(const IPDiffServices& srv) {
  ip_hdr_->ip_tos = srv;
}

IPType
IPPacket::protocol() const {
  return ip_hdr_->ip_p;
}

void
IPPacket::protocolIs(const IPType& protocol) {
  ip_hdr_->ip_p = protocol;
}

uint16_t
IPPacket::identification() const {
  return ntohs(ip_hdr_->ip_id);
}

void
IPPacket::identificationIs(uint16_t id) {
  ip_hdr_->ip_id = htons(id);
}

IPFlags
IPPacket::flags() const {
  return (uint8_t)(ntohs(ip_hdr_->ip_fl_off) >> 13);
}

void
IPPacket::flagsAre(const IPFlags& flags) {
  uint16_t shifted_flags = flags << 13;
  uint16_t masked_offset = ntohs(ip_hdr_->ip_fl_off) & 0x1FFF;
  ip_hdr_->ip_fl_off = htons(shifted_flags | masked_offset);
}

IPFragmentOffset
IPPacket::fragmentOffset() const {
  return ntohs(ip_hdr_->ip_fl_off) & 0x1FFF;
}

void
IPPacket::fragmentOffsetIs(const IPFragmentOffset& off) {
  uint16_t shifted_flags = ntohs(ip_hdr_->ip_fl_off) & 0xE000;
  uint16_t masked_offset = off & 0x1FFF;
  ip_hdr_->ip_fl_off = htons(shifted_flags | masked_offset);
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
  return ip_hdr_->ip_src;
}

void
IPPacket::srcIs(const IPv4Addr& src) {
  ip_hdr_->ip_src = src.nbo();
}

IPv4Addr
IPPacket::dst() const {
  return ip_hdr_->ip_dst;
}

void
IPPacket::dstIs(const IPv4Addr& dst) {
  ip_hdr_->ip_dst = dst.nbo();
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
