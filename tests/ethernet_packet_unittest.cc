#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <net/ethernet.h>
#include <string>

#include "arp_packet.h"
#include "fwk/buffer.h"
#include "ethernet_packet.h"
#include "ip_packet.h"
#include "unknown_packet.h"

using std::string;


TEST(EthernetAddrTest, ConstructZero) {
  EthernetAddr addr;
  EXPECT_EQ("00:00:00:00:00:00", (string)addr);
}


TEST(EthernetAddrTest, ConstructBytes) {
  uint8_t bytes[] = { 0xC0, 0xFF, 0xEE, 0xBA, 0xBE, 0xEE };
  EthernetAddr addr(bytes);
  EXPECT_EQ("C0:FF:EE:BA:BE:EE", (string)addr);
}


TEST(EthernetAddrTets, ConstructString) {
  // Try an uppercase MAC.
  const string& upper = "00:DE:AD:BE:EF:00";
  EthernetAddr addr(upper);
  EXPECT_EQ(upper, (string)addr);

  // Try a lowercase MAC.
  const char* const lower = "be:ef:ca:fe:ba:be";
  EthernetAddr addr2(lower);
  EXPECT_EQ("BE:EF:CA:FE:BA:BE", (string)addr2);

  // Try an invalid string.
  const char* const invalid = "invalidmac";
  EthernetAddr addr3(invalid);
  EXPECT_EQ("00:00:00:00:00:00", (string)addr3);

  // Long strings that start with MAC addresses are still invalid.
  const char* const invalid_long = "be:ef:ca:fe:ba:be_this_is_long";
  EthernetAddr addr4(invalid_long);
  EXPECT_EQ("00:00:00:00:00:00", (string)addr4);
}


class EthernetPacketTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    uint8_t src[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    uint8_t dst[] = { 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C };
    uint16_t type = htons(ETHERTYPE_ARP);  // packet should be in network order

    // Put a packet in a buffer.
    const char* payload = "This is an ethernet packet payload.";
    uint8_t pkt[ETHER_ADDR_LEN * 2 + ETHER_TYPE_LEN + strlen(payload)];
    buf_ = Fwk::Buffer::BufferNew(pkt, sizeof(pkt));

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

    // Construct packet.
    pkt_ = EthernetPacket::New(buf_, 0);
  }

  Fwk::Buffer::Ptr buf_;
  struct ether_header* header_;
  char* payload_;
  EthernetPacket::Ptr pkt_;
};


TEST_F(EthernetPacketTest, Src) {
  // Extract the source address.
  EthernetAddr src = pkt_->src();

  // Compare it with expected source.
  EXPECT_EQ(EthernetAddr(header_->ether_shost), src);

  // Change the source.
  uint8_t kNewSrc[] = { 0xDE, 0xAD, 0xC0, 0xFF, 0xEE, 0x00 };
  pkt_->srcIs(EthernetAddr(kNewSrc));
  EXPECT_EQ(EthernetAddr(kNewSrc), pkt_->src());
}


TEST_F(EthernetPacketTest, Dst) {
  // Extract the destination address.
  EthernetAddr dst = pkt_->dst();

  // Compare it with the expected destination.
  EXPECT_EQ(EthernetAddr(header_->ether_dhost), dst);

  // Change the destination.
  uint8_t kNewDst[] = { 0xC0, 0xFF, 0xEE, 0xBA, 0xBE, 0xCC };
  pkt_->dstIs(EthernetAddr(kNewDst));
  EXPECT_EQ(EthernetAddr(kNewDst), pkt_->dst());
}


TEST_F(EthernetPacketTest, EtherType) {
  // Ensure Ethernet type is exported properly.
  uint16_t expected = ntohs(header_->ether_type);
  ASSERT_EQ(expected, (uint16_t)pkt_->type());

  // Set the Ethernet type to IP.
  pkt_->typeIs(EthernetPacket::kIP);
  EXPECT_EQ(ETHERTYPE_IP, pkt_->type());
  EXPECT_EQ("IP", pkt_->typeName());

  // Set the Ethernet type to ARP.
  pkt_->typeIs(EthernetPacket::kARP);
  EXPECT_EQ(ETHERTYPE_ARP, pkt_->type());
  EXPECT_EQ("ARP", pkt_->typeName());

  // Set the Ethernet type to an unknown value.
  pkt_->typeIs(EthernetPacket::kUnknown);
  EXPECT_EQ(EthernetPacket::kUnknown, pkt_->type());
  EXPECT_EQ("unknown", pkt_->typeName());
}


TEST_F(EthernetPacketTest, PayloadARP) {
  // Force the type to be ARP.
  pkt_->typeIs(EthernetPacket::kARP);

  // Extract the excapsulated packet.
  Packet::Ptr payload = pkt_->payload();
  ASSERT_NE((Packet*)NULL, payload.ptr());

  // Ensure we extracted the expected payload.
  EXPECT_EQ((uint8_t*)payload_, payload->data());

  // Ensure we got the correct type.
  EXPECT_TRUE(dynamic_cast<ARPPacket*>(payload.ptr()));
}


TEST_F(EthernetPacketTest, PayloadIP) {
  // Force the type to be IP.
  pkt_->typeIs(EthernetPacket::kIP);

  // Extract the excapsulated packet.
  Packet::Ptr payload = pkt_->payload();
  ASSERT_NE((Packet*)NULL, payload.ptr());

  // Ensure we extracted the expected payload.
  EXPECT_EQ((uint8_t*)payload_, payload->data());

  // Ensure we got the correct type.
  EXPECT_TRUE(dynamic_cast<IPPacket*>(payload.ptr()));
}


TEST_F(EthernetPacketTest, PayloadUnknown) {
  // Force the type to be unknown.
  pkt_->typeIs(EthernetPacket::kUnknown);

  // Extract the excapsulated packet.
  Packet::Ptr payload = pkt_->payload();
  ASSERT_NE((Packet*)NULL, payload.ptr());

  // Ensure we extracted the expected payload.
  EXPECT_EQ((uint8_t*)payload_, payload->data());

  // Ensure we got the correct type.
  EXPECT_TRUE(dynamic_cast<UnknownPacket*>(payload.ptr()));
}
