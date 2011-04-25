#include "gre_packet.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include "ethernet_packet.h"
#include "fwk/buffer.h"
#include "interface.h"
#include "ip_packet.h"


GREPacket::GREPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      gre_hdr_((struct GREHeader *)offsetAddress(0)) { }


void GREPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
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

  // TODO(ms): Check buffer length here.

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
