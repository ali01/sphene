#include "gtest/gtest.h"

#include <cstring>

#include "fwk/buffer.h"

#include "ethernet_packet.h"


TEST(PacketTest, Size) {
  const char *data = "01234567890123456789";  // 20 bytes

  Fwk::Buffer::Ptr buf = Fwk::Buffer::BufferNew(data, strlen(data));
  EthernetPacket::Ptr pkt = EthernetPacket::EthernetPacketNew(buf, 0);

  ASSERT_EQ(buf, pkt->buffer());
  ASSERT_EQ(strlen(data), pkt->len());
}
