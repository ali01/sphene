#include "gtest/gtest.h"

#include "fwk/buffer.h"

#include "ethernet_packet.h"


TEST(PacketTest, Construct) {
  const char *data = "this is my string";

  Fwk::Buffer::Ptr buf = Fwk::Buffer::BufferNew(data, sizeof(data));
  EthernetPacket::Ptr pkt = EthernetPacket::EthernetPacketNew(buf, 0);

  ASSERT_EQ(buf, pkt->buffer());
}
