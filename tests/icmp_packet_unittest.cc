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
    uint8_t icmp_packet[] = { 0xFF, 0xFF, 0xFF, 0xFF,
                              0xFF, 0xFF, 0xFF, 0xFF };  // TODO(ms): this.

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


}


TEST_F(ICMPPacketTest, code) {


}


TEST_F(ICMPPacketTest, checksum) {


}


// TODO(ms): Need tests for subtype ICMP packets.
// TODO(ms): Need tests for "rest of header"
