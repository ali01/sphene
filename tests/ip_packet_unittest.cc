#include "gtest/gtest.h"

#include <cstring>
#include <inttypes.h>

#include "fwk/buffer.h"
#include "ip_packet.h"


class IPPacketTest : public ::testing::Test {
 protected:
  IPPacketTest() {
    /* Sample IP packet buffer in network byte order */
    uint8_t packet_buffer[] = { 0xAA, 0xBB, 0xCC, /* irrelevant bytes before */
                                0x45, 0x00, 0x00, 0x78, /* first word */
                                0x7c, 0x83, 0x00, 0x00, /* second word */
                                0x6f, 0x11, 0xaf, 0xdc, /* third word */
                                0x8d, 0x59, 0xe2, 0x92, /* fourth word */
                                0xab, 0x42, 0x03, 0xe7, /* fifth word */
                                0xDD, 0xEE, 0xFF /* irrelevant bytes after */ };

    Fwk::Buffer::Ptr buf_ =
      Fwk::Buffer::BufferNew(packet_buffer, sizeof(packet_buffer));
    pkt_ = IPPacket::IPPacketNew(buf_, 3);
  }

  IPPacket::Ptr pkt_;
};


TEST_F(IPPacketTest, ip_v_hl) {
  IPVersion version = pkt_->version();
  EXPECT_EQ(version, 4);

  IPHeaderLength header_length = pkt_->headerLength();
  EXPECT_EQ(header_length, 5);

  pkt_->versionIs(6);
  pkt_->headerLengthIs(8);

  version = pkt_->version();
  header_length = pkt_->headerLength();

  EXPECT_EQ(version, 6);
  EXPECT_EQ(header_length, 8);
}
