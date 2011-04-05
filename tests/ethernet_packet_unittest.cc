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
    memcpy(header_.ether_shost, src, sizeof(src));
    memcpy(header_.ether_dhost, dst, sizeof(dst));
    header_.ether_type = ETHERTYPE_IP;

    src_ = src;
    dst_ = dst;

    // Put the Ethernet header in a buffer.
    buf_ = Buffer::BufferNew(&header_, sizeof(header_));
  }

  EthernetAddr src_;
  EthernetAddr dst_;
  struct ether_header header_;
  Buffer::Ptr buf_;
};


TEST_F(EthernetPacketTest, Addresses) {
  // Construct packet.
  EthernetPacket pkt(buf_, 0);

  // Extract the source address.
  EthernetAddr src = pkt.src();

  // Compare it with expected source.
  EXPECT_EQ(src_, src);

  // Change the source.
  uint8_t kNewSrc[] = { 0xDE, 0xAD, 0xC0, 0xFF, 0xEE, 0x00 };
  pkt.srcIs(EthernetAddr(kNewSrc));
  EXPECT_EQ(EthernetAddr(kNewSrc), pkt.src());

  // Extract the destination address.
  EthernetAddr dst = pkt.dst();

  // Compare it with the expected destination.
  EXPECT_EQ(dst_, dst);
}


TEST_F(EthernetPacketTest, EtherType) {
  EthernetPacket pkt(buf_, 0);

  // Ensure Ethernet type is exported properly.
  ASSERT_EQ(header_.ether_type, pkt.type());

  // Set the Ethernet type to IP.
  pkt.typeIs(ETHERTYPE_IP);
  EXPECT_EQ(ETHERTYPE_IP, pkt.type());

  // Set the Ethernet type to ARP.
  pkt.typeIs(ETHERTYPE_ARP);
  EXPECT_EQ(ETHERTYPE_ARP, pkt.type());
}
