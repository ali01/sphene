#include "ip_packet.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include "gre_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "packet_buffer.h"
#include "unknown_packet.h"

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

/* Static members. */
const uint8_t IPPacket::kDefaultTTL;
const size_t IPPacket::kHeaderSize = sizeof(struct ip_hdr);
const IPVersion IPPacket::kVersion;

/* IPPacket */

IPPacket::Ptr
IPPacket::NewDefault(PacketBuffer::Ptr buffer,
                     uint16_t packet_len,
                     IPType protocol,
                     const IPv4Addr& src,
                     const IPv4Addr& dst) {
  IPPacket::Ptr pkt = new IPPacket(buffer, buffer->size() - packet_len);

  pkt->versionIs(IPPacket::kVersion);
  pkt->headerLengthIs(IPPacket::kHeaderSize / 4); /* Words, not bytes. */
  pkt->diffServicesAre(0);
  pkt->packetLengthIs(packet_len);
  pkt->identificationIs(0);
  pkt->flagsAre(0);
  pkt->fragmentOffsetIs(0);
  pkt->ttlIs(kDefaultTTL);
  pkt->protocolIs(protocol);
  pkt->srcIs(src);
  pkt->dstIs(dst);
  pkt->checksumReset();

  return pkt;
}

IPPacket::IPPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      ip_hdr_((struct ip_hdr *)offsetAddress(0)) { }


void IPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}

bool
IPPacket::valid() const {
  if (len() < sizeof(struct ip_hdr) || len() < packetLength())
    return false;

  if (headerLength() < 5)  // in words, not bytes
    return false;

  if (version() != 4)
    return false;

  if (!checksumValid())
    return false;

  return true;
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

// TODO(ms): THIS NEEDS SOME DOCUMENTATION.
//   Specifically, the length here is NOT IN BYTES AS ONE MIGHT EXPECT.
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

IPPacket::IPType
IPPacket::protocol() const {
  return (IPPacket::IPType)(ip_hdr_->ip_p);
}

void
IPPacket::protocolIs(const IPType& protocol) {
  uint8_t p = (uint8_t)protocol;
  ip_hdr_->ip_p = p;
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
  if (ip_hdr_->ip_ttl > dec_amount)
    ip_hdr_->ip_ttl -= dec_amount;
  else
    ip_hdr_->ip_ttl = 0;
}

IPv4Addr
IPPacket::src() const {
  return ntohl(ip_hdr_->ip_src);
}

void
IPPacket::srcIs(const IPv4Addr& src) {
  ip_hdr_->ip_src = src.nbo();
}

IPv4Addr
IPPacket::dst() const {
  return ntohl(ip_hdr_->ip_dst);
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

uint16_t
IPPacket::checksumReset() {
  checksumIs(0);
  checksumIs(compute_cksum(ip_hdr_, sizeof(struct ip_hdr)));
  return checksum();
}

bool
IPPacket::checksumValid() const {
  IPPacket *self = const_cast<IPPacket*>(this);
  uint16_t pkt_cksum, actual_cksum;
  pkt_cksum = checksum();
  actual_cksum = self->checksumReset();
  self->checksumIs(pkt_cksum);
  return pkt_cksum == actual_cksum;
}

// TODO(ms): Need tests for this.
Packet::Ptr
IPPacket::payload() {
  uint16_t payload_offset = bufferOffset() + headerLen();
  Packet::Ptr pkt;

  switch (protocol()) {
    case kICMP:
      pkt = ICMPPacket::ICMPPacketNew(buffer(), payload_offset);
      break;
    case kGRE:
      pkt = GREPacket::GREPacketNew(buffer(), payload_offset);
      break;
    default:
      pkt = UnknownPacket::UnknownPacketNew(buffer(), payload_offset);
      break;
  }

  // This IP packet encapsulates the payload.
  pkt->enclosingPacketIs(this);
  return pkt;
}
