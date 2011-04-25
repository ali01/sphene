#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "arp_packet.h"
#include "ethernet_packet.h"
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
                             0x08, 0x06,    // Ptype = ARP
                             0x4F, 0xD8,    // Checksum
                             0x00, 0x00,    // Reserved1
                             // The following is an ARP packet.
                             0x00, 0x01,    // Ethernet = 0x0001
                             0x08, 0x00,    // IP = 0x0800
                             0x06, 0x04,    // hlen = 6, plen = 4
                             0x00, 0x01,    // request packet
                             0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xBE,  // Sender HW
                             0x04, 0x02, 0x02, 0x01,              // Sender P
                             0xC0, 0xFF, 0xEE, 0xC0, 0xFF, 0xEE,  // Target HW
                             0x08, 0x08, 0x04, 0x04 };            // Targer P

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

  // Checksum is essentially 0 if it is not present.
  pkt_->checksumPresentIs(false);
  EXPECT_EQ(0, pkt_->checksum());

  // Changing the checksum is a no-op if the checksum present bit is 0.
  pkt_->checksumIs(0xBEEF);
  pkt_->checksumPresentIs(true);
  EXPECT_EQ(0xDEAD, pkt_->checksum());

  // Reset the checksum. Only the checksum field should have changed, so the
  // checksum should reset back to what we expect.
  EXPECT_EQ(expected, pkt_->checksumReset());
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


TEST_F(GREPacketTest, ptype) {
  // Save original checksum.
  uint16_t cksum = pkt_->checksum();

  // Query the protocol type.
  EXPECT_EQ(EthernetPacket::kARP, pkt_->protocol());

  // Change the protocol.
  pkt_->protocolIs(EthernetPacket::kIP);
  EXPECT_EQ(EthernetPacket::kIP, pkt_->protocol());

  // Reset. Only the protocol field should have changed.
  pkt_->protocolIs(EthernetPacket::kARP);
  EXPECT_EQ(cksum, pkt_->checksumReset());
}


TEST_F(GREPacketTest, reserved1) {
  // Save original checksum.
  uint16_t cksum = pkt_->checksum();

  // Check reserved1 value.
  EXPECT_EQ(0, pkt_->reserved1());

  // Set reserved1.
  pkt_->reserved1Is(0xDEAD);
  EXPECT_EQ(0xDEAD, pkt_->reserved1());

  // Reserved1 should not be present if the checksum bit is not present.
  pkt_->checksumPresentIs(false);
  EXPECT_EQ(0, pkt_->reserved1());

  // Setting the reserved1 field is a no-op if checksum bit is not present.
  pkt_->reserved1Is(0xBEEF);
  pkt_->checksumPresentIs(true);
  EXPECT_EQ(0xDEAD, pkt_->reserved1());

  // Reset.
  pkt_->reserved1Is(0);
  EXPECT_EQ(cksum, pkt_->checksumReset());
}


TEST_F(GREPacketTest, payload) {
  // Extract the excapsulated packet.
  Packet::Ptr payload = pkt_->payload();
  ASSERT_NE((Packet*)NULL, payload.ptr());

  // Ensure we extracted the expected payload.
  ASSERT_TRUE(dynamic_cast<ARPPacket*>(payload.ptr()));
  ARPPacket::Ptr arp_pkt = ARPPacket::Ptr::st_cast<ARPPacket>(payload);
  EXPECT_EQ("4.2.2.1", (string)arp_pkt->senderPAddr());
}
