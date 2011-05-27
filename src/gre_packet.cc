#include "gre_packet.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include "arp_packet.h"
#include "ethernet_packet.h"
#include "fwk/exception.h"
#include "fwk/log.h"
#include "interface.h"
#include "packet_buffer.h"
#include "unknown_packet.h"

static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("GREPacket");
const size_t GREPacket::kHeaderSize = sizeof(struct GREHeader);


GREPacket::GREPacket(const PacketBuffer::Ptr buffer,
                     const unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      gre_hdr_((struct GREHeader *)offsetAddress(0)) { }


void GREPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


// Packet validation.
bool GREPacket::valid() const {
  // Verify length.
  const size_t header_len = checksumPresent() ? 8 : 4;
  if (len() < header_len) {
    DLOG << "length is too short: " << len();
    return false;
  }

  if (reserved0() != 0 || reserved1() != 0) {
    DLOG << "reserved field non-zero";
    DLOG << "  reserved0: " << (uint32_t)reserved0();
    DLOG << "  reserved1: " << (uint32_t)reserved1();
    return false;
  }

  if (!checksumValid()) {
    DLOG << "invalid checksum";
    return false;
  }

  return true;
}


bool GREPacket::checksumPresent() const {
  // Is C bit present?
  uint16_t field = ntohs(gre_hdr_->c_resv0_ver);
  return (((field >> 15) & 0x01) == 1) ? true : false;
}


void GREPacket::checksumPresentIs(const bool value) {
  // Get current value.
  uint16_t field = ntohs(gre_hdr_->c_resv0_ver);

  // Clear the MSB.
  field &= 0x7FFF;

  // Set the MSB.
  field |= (value << 15);

  // Set field in packet.
  gre_hdr_->c_resv0_ver = htons(field);
}


uint16_t GREPacket::checksum() const {
  return (checksumPresent()) ? ntohs(gre_hdr_->cksum) : 0;
}


void GREPacket::checksumIs(const uint16_t ck) {
  // No-op if checksum bit isn't set.
  if (!checksumPresent())
    return;

  gre_hdr_->cksum = htons(ck);
}


bool GREPacket::checksumValid() const {
  if (!checksumPresent())
    return true;

  // Checksum field is present.
  uint16_t pkt_cksum = checksum();

  GREPacket* pkt = const_cast<GREPacket*>(this);
  uint16_t calculated_cksum = pkt->checksumReset();
  pkt->checksumIs(pkt_cksum);
  return (pkt_cksum == calculated_cksum);
}


uint16_t GREPacket::checksumReset() {
  checksumIs(0);
  checksumIs(computeChecksum());
  return checksum();
}


// TODO(ms): This should be moved to a util.h/util.cc pair.
uint16_t GREPacket::computeChecksum() const {
  uint32_t sum;
  unsigned int length = len();
  const uint8_t *header = (const uint8_t *)gre_hdr_;

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


uint16_t GREPacket::reserved0() const {
  uint16_t field = ntohs(gre_hdr_->c_resv0_ver);

  // First 16 bits of the header:
  //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |C|       Reserved0       | Ver |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  const int resv0_mask = (0x0FFF << 3);

  return ((field & resv0_mask) >> 3);
}


void GREPacket::reserved0Is(uint16_t value) {
  // Get current value.
  uint16_t field = ntohs(gre_hdr_->c_resv0_ver);

  // Clear Reserved0 field.
  const int resv0_mask = (0x0FFF << 3);
  field &= ~resv0_mask;

  // We will use only the lower 12 bits of value.
  value &= 0x0FFF;

  // Set Reserved0 field to value.
  field |= (value << 3);
  gre_hdr_->c_resv0_ver = htons(field);
}


uint8_t GREPacket::version() const {
  // Lower three bits.
  uint16_t field = ntohs(gre_hdr_->c_resv0_ver);
  return (field & 0x07);
}


void GREPacket::versionIs(uint8_t value) {
  // Get current value.
  uint16_t field = ntohs(gre_hdr_->c_resv0_ver);

  // Clear version field.
  field &= ~0x07;

  // Set version field.
  value &= 0x07;
  field |= value;
  gre_hdr_->c_resv0_ver = htons(field);
}


EthernetPacket::EthernetType GREPacket::protocol() const {
  return (EthernetPacket::EthernetType)ntohs(gre_hdr_->ptype);
}


void GREPacket::protocolIs(const EthernetPacket::EthernetType ptype) {
  gre_hdr_->ptype = htons(ptype);
}


uint16_t GREPacket::reserved1() const {
  return (checksumPresent()) ? ntohs(gre_hdr_->resv1) : 0;
}


void GREPacket::reserved1Is(const uint16_t value) {
  // No-op if checksum bit isn't set.
  if (!checksumPresent())
    return;

  gre_hdr_->resv1 = htons(value);
}


Packet::Ptr GREPacket::payload() {
  const uint16_t header_len = checksumPresent() ? 8 : 4;
  const uint16_t payload_offset = bufferOffset() + header_len;
  Packet::Ptr pkt;

  switch (protocol()) {
    case EthernetPacket::kARP:
      pkt = ARPPacket::ARPPacketNew(buffer(), payload_offset);
      break;
    case EthernetPacket::kIP:
      pkt = IPPacket::New(buffer(), payload_offset);
      break;
    default:
      pkt = UnknownPacket::UnknownPacketNew(buffer(), payload_offset);
      break;
  }

  // Chain the packets together.
  pkt->enclosingPacketIs(this);
  return pkt;
}
