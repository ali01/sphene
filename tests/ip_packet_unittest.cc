#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <inttypes.h>

#include "ip_packet.h"
#include "packet_buffer.h"


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

    PacketBuffer::Ptr buf_ =
      PacketBuffer::New(packet_buffer, sizeof(packet_buffer));
    pkt_ = IPPacket::New(buf_,
                                 buf_->size() - sizeof(packet_buffer) + 3);
    ip_hdr_ = pkt_->data(); /* ptr to beginning of IP header */
  }

  const uint8_t *ip_hdr_;
  IPPacket::Ptr pkt_;
};

class IPv4AddrTest : public ::testing::Test {
 protected:
  IPv4AddrTest()
      : ip_val_(0xabcdef42),
        ip_val_nbo_(htonl(ip_val_)),
        addr_(ip_val_) {}

  uint32_t ip_val_;
  uint32_t ip_val_nbo_;
  IPv4Addr addr_;
};


/* IP Packet */

TEST_F(IPPacketTest, ip_v_hl) {
  EXPECT_EQ(pkt_->version(), 4);
  EXPECT_EQ(pkt_->version(), ip_hdr_[0] >> 4);

  EXPECT_EQ(pkt_->headerLength(), 5);
  EXPECT_EQ(pkt_->headerLength(), ip_hdr_[0] & 0x0F);

  pkt_->versionIs(6);
  EXPECT_EQ(pkt_->version(), 6);
  EXPECT_EQ(pkt_->version(), ip_hdr_[0] >> 4);

  pkt_->headerLengthIs(8);
  EXPECT_EQ(pkt_->headerLength(), 8);
  EXPECT_EQ(pkt_->headerLength(), ip_hdr_[0] & 0x0F);
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
  EXPECT_EQ(pkt_->flags(), ip_hdr_[6] >> 5);

  pkt_->flagsAre(IPPacket::kIP_RF);
  EXPECT_EQ(pkt_->flags(), IPPacket::kIP_RF);
  EXPECT_EQ(pkt_->flags(), ip_hdr_[6] >> 5);

  pkt_->flagsAre(IPPacket::kIP_DF);
  EXPECT_EQ(pkt_->flags(), IPPacket::kIP_DF);
  EXPECT_EQ(pkt_->flags(), ip_hdr_[6] >> 5);

  pkt_->flagsAre(IPPacket::kIP_MF);
  EXPECT_EQ(pkt_->flags(), IPPacket::kIP_MF);
  EXPECT_EQ(pkt_->flags(), ip_hdr_[6] >> 5);

  pkt_->flagsAre(IPPacket::kIP_RF | IPPacket::kIP_MF);
  EXPECT_EQ(pkt_->flags(), IPPacket::kIP_RF | IPPacket::kIP_MF);
  EXPECT_EQ(pkt_->flags(), ip_hdr_[6] >> 5);

  /* fragment offset */
  EXPECT_EQ(pkt_->fragmentOffset(), 0);

  pkt_->fragmentOffsetIs(0xAD);
  EXPECT_EQ(0xAD, pkt_->fragmentOffset());

  /* retesting flags */
  EXPECT_EQ(IPPacket::kIP_RF | IPPacket::kIP_MF, pkt_->flags());
}

TEST_F(IPPacketTest, ip_ttl) {
  EXPECT_EQ(111, pkt_->ttl());

  pkt_->ttlIs(147);
  EXPECT_EQ(147, pkt_->ttl());
}

TEST_F(IPPacketTest, ip_p) {
  EXPECT_EQ(IPPacket::kUDP, pkt_->protocol());

  pkt_->protocolIs(IPPacket::kICMP); /* ICMP */
  EXPECT_EQ(IPPacket::kICMP, pkt_->protocol());
}

TEST_F(IPPacketTest, ip_sum) {
  EXPECT_EQ(pkt_->checksum(), 0xafdc);
  EXPECT_TRUE(pkt_->checksumValid());

  pkt_->checksumReset();
  EXPECT_EQ(pkt_->checksum(), 0xafdc);

  pkt_->checksumIs(0x4242);
  EXPECT_EQ(pkt_->checksum(), 0x4242);
}

TEST_F(IPPacketTest, ip_src_dst) {
  EXPECT_EQ(pkt_->src(), 0x8d59e292);
  EXPECT_EQ(pkt_->dst(), 0xab4203e7);

  std::string ip_one_str = "192.168.0.1";
  IPv4Addr ip_one(ip_one_str);
  pkt_->srcIs(ip_one_str);
  EXPECT_EQ(pkt_->src(), ip_one);
  EXPECT_EQ(*(uint32_t *)&ip_hdr_[12], ip_one.nbo());

  std::string ip_two_str = "10.4.0.1";
  IPv4Addr ip_two(ip_two_str);
  pkt_->dstIs(ip_two_str);
  EXPECT_EQ(pkt_->dst(), ip_two);
  EXPECT_EQ(*(uint32_t *)&ip_hdr_[16], ip_two.nbo());
}


/* IPv4Addr */

TEST_F(IPv4AddrTest, int_construction) {
  uint32_t val = addr_.value();
  EXPECT_EQ(val, ip_val_);
  EXPECT_EQ(ip_val_nbo_, addr_.nbo());

  EXPECT_TRUE(addr_ == ip_val_);
  EXPECT_TRUE(ip_val_ == addr_.value());
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
