#include "gtest/gtest.h"

#include <arpa/inet.h>
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
    packet_buffer_ = pkt_->buffer();
  }

  const uint8_t *packet_buffer_;
  IPPacket::Ptr pkt_;
};

class IPv4AddrTest : public ::testing::Test {
 protected:
  IPv4AddrTest()
      : ip_val_(0xabcdef42),
        ip_val_nbo_(htonl(ip_val_)),
        addr_(ip_val_nbo_) {}

  uint32_t ip_val_;
  uint32_t ip_val_nbo_;
  IPv4Addr addr_;
};


/* IP Packet */

TEST_F(IPPacketTest, ip_v_hl) {
  EXPECT_EQ(pkt_->version(), 4);
  EXPECT_EQ(pkt_->version(), packet_buffer_[0] >> 4);

  EXPECT_EQ(pkt_->headerLength(), 5);
  EXPECT_EQ(pkt_->headerLength(), packet_buffer_[0] & 0x0F);

  pkt_->versionIs(6);
  EXPECT_EQ(pkt_->version(), 6);
  EXPECT_EQ(pkt_->version(), packet_buffer_[0] >> 4);

  pkt_->headerLengthIs(8);
  EXPECT_EQ(pkt_->headerLength(), 8);
  EXPECT_EQ(pkt_->headerLength(), packet_buffer_[0] & 0x0F);
}

TEST_F(IPPacketTest, ip_tos) {
  IPDiffServices diff_services = pkt_->diffServices();
  EXPECT_EQ(diff_services, 0);

  pkt_->diffServicesAre(0xAD);
  diff_services = pkt_->diffServices();
  EXPECT_EQ(diff_services, 0xad);
}

TEST_F(IPPacketTest, ip_len) {
  uint16_t packet_len = pkt_->packetLength();
  EXPECT_EQ(packet_len, 120);

  pkt_->packetLengthIs(0xbeef);
  packet_len = pkt_->packetLength();
  EXPECT_EQ(packet_len, 0xbeef);
}

TEST_F(IPPacketTest, ip_id) {
  uint16_t id = pkt_->identification();
  EXPECT_EQ(id, 0x7c83);

  pkt_->identificationIs(0xdead);
  id = pkt_->identification();
  EXPECT_EQ(id, 0xdead);
}

TEST_F(IPPacketTest, ip_fl_off) {
  /* IP Flags */
  EXPECT_EQ(pkt_->flags(), 0);
  EXPECT_EQ(pkt_->flags(), packet_buffer_[6] >> 5);

  pkt_->flagsAre(IPPacket::IP_RF);
  EXPECT_EQ(pkt_->flags(), IPPacket::IP_RF);
  EXPECT_EQ(pkt_->flags(), packet_buffer_[6] >> 5);

  pkt_->flagsAre(IPPacket::IP_DF);
  EXPECT_EQ(pkt_->flags(), IPPacket::IP_DF);
  EXPECT_EQ(pkt_->flags(), packet_buffer_[6] >> 5);

  pkt_->flagsAre(IPPacket::IP_MF);
  EXPECT_EQ(pkt_->flags(), IPPacket::IP_MF);
  EXPECT_EQ(pkt_->flags(), packet_buffer_[6] >> 5);

  pkt_->flagsAre(IPPacket::IP_RF | IPPacket::IP_MF);
  EXPECT_EQ(pkt_->flags(), IPPacket::IP_RF | IPPacket::IP_MF);
  EXPECT_EQ(pkt_->flags(), packet_buffer_[6] >> 5);

  /* fragment offset */
  EXPECT_EQ(pkt_->fragmentOffset(), 0);

  pkt_->fragmentOffsetIs(0xAD);
  EXPECT_EQ(0xAD, pkt_->fragmentOffset());

  /* retesting flags */
  EXPECT_EQ(IPPacket::IP_RF | IPPacket::IP_MF, pkt_->flags());
}

TEST_F(IPPacketTest, ip_ttl) {
  EXPECT_EQ(111, pkt_->ttl());

  pkt_->ttlIs(147);
  EXPECT_EQ(147, pkt_->ttl());
}

TEST_F(IPPacketTest, ip_p) {
  EXPECT_EQ(0x11, pkt_->protocol()); /* UDP Expected */

  pkt_->protocolIs(0x01); /* ICMP */
  EXPECT_EQ(1, pkt_->protocol());
}

TEST_F(IPPacketTest, ip_sum) {
  EXPECT_EQ(pkt_->checksum(), 0xafdc);

  pkt_->checksumReset();
  EXPECT_EQ(pkt_->checksum(), 0xafdc);

  pkt_->checksumIs(0x4242);
  EXPECT_EQ(pkt_->checksum(), 0x4242);
}

TEST_F(IPPacketTest, ip_src) {
  EXPECT_EQ((uint32_t)pkt_->src(), 0x8d59e292);
}


/* IPv4Addr */

TEST_F(IPv4AddrTest, int_construction) {
  uint32_t val = addr_;
  EXPECT_EQ(val, ip_val_);
  EXPECT_EQ(ip_val_nbo_, addr_.nbo());

  EXPECT_TRUE(addr_ == ip_val_);
  EXPECT_TRUE(ip_val_ == addr_);
}

TEST_F(IPv4AddrTest, str_construction) {
  std::string str = addr_; /* address implicitly casted into std::str */
  const char *c_str = (const char *)"171.205.239.66";

  EXPECT_EQ(str, c_str);

  IPv4Addr addr_from_str(str);
  EXPECT_EQ(addr_from_str, ip_val_);
  EXPECT_EQ(addr_from_str.nbo(), ip_val_nbo_);

  IPv4Addr addr_from_c_str(c_str);
  EXPECT_EQ(addr_from_c_str, ip_val_);
  EXPECT_EQ(addr_from_c_str.nbo(), ip_val_nbo_);
}
