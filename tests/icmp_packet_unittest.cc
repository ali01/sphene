#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "icmp_packet.h"
#include "fwk/buffer.h"
#include "fwk/exception.h"

using std::string;


namespace {

struct ICMPHeader {
  uint8_t type;
  uint8_t code;
  uint16_t cksum;
} __attribute__((packed));

}  // namespace


class ICMPPacketTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Everything here is in NBO.
    // Type 8 code 0 is a ping request.
    uint8_t icmp_packet[] = { 0x08,        // Type
                              0x00,        // Code
                              0x1D, 0xEF,  // Checksum
                              0x72, 0x70,  // Identifier
                              0x00, 0x00,  // Sequence number
                              0x4D, 0xA3, 0x95, 0xD4, 0x00, 0x08, 0x99, 0x1D,
                              0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                              0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                              0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                              0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                              0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                              0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };

    // Put a packet in a buffer.
    buf_ = Fwk::Buffer::BufferNew(icmp_packet, sizeof(icmp_packet));

    // Update header pointer.
    header_ = (struct ICMPHeader*)buf_->data();

    // Construct ICMPPacket from buffer.
    pkt_ = ICMPPacket::ICMPPacketNew(buf_, 0);
  }

  Fwk::Buffer::Ptr buf_;
  struct ICMPHeader* header_;
  ICMPPacket::Ptr pkt_;
};


TEST_F(ICMPPacketTest, type) {
  // Ensure we retrieve the type correctly.
  EXPECT_EQ(header_->type, pkt_->type());

  // Ensure we can set the type.
  pkt_->typeIs(ICMPPacket::kEchoReply);
  EXPECT_EQ(ICMPPacket::kEchoReply, pkt_->type());
}


TEST_F(ICMPPacketTest, code) {
  // Ensure we retrieve the code correctly.
  EXPECT_EQ(header_->code, pkt_->code());

  // Ensure we can set the code.
  pkt_->codeIs(0xFF);
  EXPECT_EQ(0xFF, pkt_->code());
}


TEST_F(ICMPPacketTest, checksum) {
  const uint16_t expected = ntohs(header_->cksum);
  EXPECT_EQ(expected, pkt_->checksum());

  // Manually set checksum.
  pkt_->checksumIs(0xDEAD);
  EXPECT_EQ(0xDEAD, pkt_->checksum());

  // Recompute checksum.
  pkt_->checksumIs(0);
  pkt_->checksumReset();
  EXPECT_EQ(expected, pkt_->checksum());
}


// TODO(ms): Need tests for subtype ICMP packets.
// TODO(ms): Need tests for "rest of header"
