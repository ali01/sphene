#include "gtest/gtest.h"

#include <cstring>
#include <inttypes.h>
#include <net/ethernet.h>
#include "fwk/buffer.h"
#include "ethernet_packet.h"


class EthernetPacketTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    uint8_t src[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    uint8_t dst[] = { 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C };
    uint16_t type = ETHERTYPE_IP;

    // Put a packet in a buffer.
    const char* payload = "This is an ethernet packet payload.";
    uint8_t pkt[ETHER_ADDR_LEN * 2 + ETHER_TYPE_LEN + strlen(payload)];
    buf_ = Buffer::BufferNew(pkt, sizeof(pkt));

    // Update header pointer.
    header_ = (struct ether_header*)buf_->data();

    // Set fields in the header.
    memcpy(header_->ether_shost, src, sizeof(src));
    memcpy(header_->ether_dhost, dst, sizeof(dst));
    header_->ether_type = type;

    // Update payload pointer.
    payload_ = (char*)((uint8_t*)header_ + ETHER_ADDR_LEN * 2 + ETHER_TYPE_LEN);

    // Copy in payload.
    memcpy(payload_, payload, strlen(payload));
  }

  Buffer::Ptr buf_;
  struct ether_header* header_;
  char* payload_;
};


TEST_F(EthernetPacketTest, Src) {
  // Construct packet.
  EthernetPacket pkt(buf_, 0);

  // Extract the source address.
  EthernetAddr src = pkt.src();

  // Compare it with expected source.
  EXPECT_EQ(EthernetAddr(header_->ether_shost), src);

  // Change the source.
  uint8_t kNewSrc[] = { 0xDE, 0xAD, 0xC0, 0xFF, 0xEE, 0x00 };
  pkt.srcIs(EthernetAddr(kNewSrc));
  EXPECT_EQ(EthernetAddr(kNewSrc), pkt.src());
}


TEST_F(EthernetPacketTest, Dst) {
  EthernetPacket pkt(buf_, 0);

  // Extract the destination address.
  EthernetAddr dst = pkt.dst();

  // Compare it with the expected destination.
  EXPECT_EQ(EthernetAddr(header_->ether_dhost), dst);

  // Change the destination.
  uint8_t kNewDst[] = { 0xC0, 0xFF, 0xEE, 0xBA, 0xBE, 0xCC };
  pkt.dstIs(EthernetAddr(kNewDst));
  EXPECT_EQ(EthernetAddr(kNewDst), pkt.dst());
}


TEST_F(EthernetPacketTest, EtherType) {
  EthernetPacket pkt(buf_, 0);

  // Ensure Ethernet type is exported properly.
  ASSERT_EQ(header_->ether_type, pkt.type());

  // Set the Ethernet type to IP.
  pkt.typeIs(ETHERTYPE_IP);
  EXPECT_EQ(ETHERTYPE_IP, pkt.type());

  // Set the Ethernet type to ARP.
  pkt.typeIs(ETHERTYPE_ARP);
  EXPECT_EQ(ETHERTYPE_ARP, pkt.type());
}
