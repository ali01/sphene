#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "fwk/buffer.h"
#include "ip_packet.h"

using std::string;


namespace {

struct ARPHeader {
  uint16_t htype;                        // hardware type
  uint16_t ptype;                        // protocol type
  uint8_t  hlen;                         // hardware address length
  uint8_t  plen;                         // protocol address length
  uint16_t oper;                         // ARP operation
  uint8_t  sha[EthernetAddr::kAddrLen];  // sender hardware address
  uint32_t spa;                          // sender protocol address
  uint8_t  tha[EthernetAddr::kAddrLen];  // target hardware address
  uint32_t tpa;                          // target protocol address
} __attribute__((packed));

}  // namespace


class ARPPacketTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Everything here is in NBO.
    uint8_t arp_packet[] = { 0x00, 0x01,  // Ethernet = 0x0001
                             0x08, 0x00,  // IP = 0x0800
                             0x06, 0x04,  // hlen = 6, plen = 4
                             0x00, 0x01,  // request packet
                             0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xBE,
                             0x04, 0x02, 0x02, 0x01,
                             0xC0, 0xFF, 0xEE, 0xC0, 0xFF, 0xEE,
                             0x08, 0x08, 0x04, 0x04 };

    // Put a packet in a buffer.
    buf_ = Fwk::Buffer::BufferNew(arp_packet, sizeof(arp_packet));

    // Update header pointer.
    header_ = (struct ARPHeader*)buf_->data();

    // Construct ARPPacket from buffer.
    pkt_ = ARPPacket::ARPPacketNew(buf_, 0);
  }

  Fwk::Buffer::Ptr buf_;
  struct ARPHeader* header_;
  ARPPacket::Ptr pkt_;
};


TEST_F(ARPPacketTest, operation) {
  // Ensure the current operation is 'request'.
  EXPECT_EQ(ARPPacket::kRequest, pkt_->operation());

  // Change the operation to 'reply'.
  pkt_->operationIs(ARPPacket::kReply);

  // Ensure that it has changed.
  EXPECT_EQ(ARPPacket::kReply, pkt_->operation());
}


TEST_F(ARPPacketTest, senderHWAddr) {
  // Ensure our packet has the expected sender hardware address.
  const string& expected = "DE:AD:BE:EF:BA:BE";
  EXPECT_EQ(expected, (string)pkt_->senderHWAddr());

  // Change the sender hardware address.
  const string& new_addr = "CA:FE:BA:BE:00:00";
  pkt_->senderHWAddrIs(new_addr);

  // Ensure that it changed.
  EXPECT_EQ(new_addr, (string)pkt_->senderHWAddr());
}
