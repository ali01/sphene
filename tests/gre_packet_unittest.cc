#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "gre_packet.h"
#include "fwk/buffer.h"
#include "fwk/exception.h"

using std::string;


namespace {

struct GREHeader {
  uint16_t c_resv0_ver;  // C bit, Reserved0, Ver
  uint16_t ptype;        // protocol type
  uint16_t cksum;        // checksum of header and payload (optional)
  uint16_t resv1;        // Reserved1 (optional)
} __attribute__((packed));

}  // namespace


class GREPacketTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    uint8_t gre_packet[] = { 0x80,          // Checksum is present
                             0x00,          // Version = 0
                             0x08, 0x00,    // Ptype = IP
                             0x77, 0xFF,    // Checksum
                             0x00, 0x00 };  // Reserved1

    // Put a packet in a buffer.
    buf_ = Fwk::Buffer::BufferNew(gre_packet, sizeof(gre_packet));

    // Update header pointer.
    header_ = (struct GREHeader*)buf_->data();

    // Construct GREPacket from buffer.
    pkt_ = GREPacket::GREPacketNew(buf_, 0);
  }

  Fwk::Buffer::Ptr buf_;
  struct GREHeader* header_;
  GREPacket::Ptr pkt_;
};


TEST_F(GREPacketTest, checksumPresent) {
  // Checksum bit is enabled.
  EXPECT_TRUE(pkt_->checksumPresent());

  // Save previous checksum.
  uint16_t ck = pkt_->checksumReset();

  // Disable the C bit.
  pkt_->checksumPresentIs(false);
  EXPECT_FALSE(pkt_->checksumPresent());

  // Re-enable the C bit.
  pkt_->checksumPresentIs(true);
  EXPECT_TRUE(pkt_->checksumPresent());

  // Checksum should not have changed after both operations.
  EXPECT_EQ(ck, pkt_->checksumReset());
}


TEST_F(GREPacketTest, checksum) {
  // Retrieve checksum from packet.
  uint16_t expected = ntohs(header_->cksum);
  EXPECT_EQ(expected, pkt_->checksum());

  // Change the checksum.
  pkt_->checksumIs(0xDEAD);
  EXPECT_EQ(0xDEAD, pkt_->checksum());

  // Reset the checksum. Only the checksum field should have changed, so the
  // checksum should reset back to what we expect.
  EXPECT_EQ(expected, pkt_->checksumReset());

  // Checksum is essentially 0 if it is not present.
  pkt_->checksumPresentIs(false);
  EXPECT_EQ(0, pkt_->checksum());
}


TEST_F(GREPacketTest, checksumValid) {
  // Check validity of checksum.
  EXPECT_TRUE(pkt_->checksumValid());

  // Modify checksum to bogus value.
  pkt_->checksumIs(0xDEAD);
  EXPECT_FALSE(pkt_->checksumValid());

  // Say the checksum isn't present. Checksum is now automatically valid.
  pkt_->checksumPresentIs(false);
  EXPECT_TRUE(pkt_->checksumValid());
}


TEST_F(GREPacketTest, checksumReset) {
  uint16_t expected = pkt_->checksum();

  // Resetting the checksum should not change the checksum.
  EXPECT_EQ(expected, pkt_->checksumReset());

  // If the checksum isn't present, resetting is a no-op.
  pkt_->checksumPresentIs(false);
  EXPECT_EQ(0, pkt_->checksumReset());
}


TEST_F(GREPacketTest, reserved0) {
  // Save original checksum.
  uint16_t cksum = pkt_->checksum();

  // Query the reserved0 bits.
  EXPECT_EQ(0, pkt_->reserved0());

  // Set the reserved0 bits.
  pkt_->reserved0Is(0x0FFF);  // 12 bits
  EXPECT_EQ(0x0FFF, pkt_->reserved0());

  // Try to set more than 12 bits.
  pkt_->reserved0Is(0xFFFF);
  EXPECT_EQ(0x0FFF, pkt_->reserved0());

  // Reset. Only the reserved0 field should have changed.
  pkt_->reserved0Is(0);
  EXPECT_EQ(cksum, pkt_->checksumReset());
}


TEST_F(GREPacketTest, version) {
  // Save original checksum.
  uint16_t cksum = pkt_->checksum();

  // Query the version bits.
  EXPECT_EQ(0, pkt_->version());

  // Set the version bits.
  pkt_->versionIs(0x07);  // 3 bits
  EXPECT_EQ(0x07, pkt_->version());

  // Try to set more than 3 bits.
  pkt_->versionIs(0xFF);
  EXPECT_EQ(0x07, pkt_->version());

  // Reset. Only the version field should have changed.
  pkt_->versionIs(0);
  EXPECT_EQ(cksum, pkt_->checksumReset());
}
