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

    src_ = src;
    dst_ = dst;

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

  // Extract the destination address.
  EthernetAddr dst = pkt.dst();

  // Compare it with the expected destination.
  EXPECT_EQ(dst_, dst);
}
