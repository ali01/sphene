#include "gtest/gtest.h"

#include <cstring>

#include "fwk/buffer.h"

#include "ethernet_packet.h"


TEST(PacketTest, Construct) {
  const char *data = "01234567890123456789";  // 20 bytes

  Fwk::Buffer::Ptr buf = Fwk::Buffer::BufferNew(data, strlen(data));
  EthernetPacket::Ptr pkt = EthernetPacket::EthernetPacketNew(buf, 0);

  ASSERT_EQ(buf, pkt->buffer());
}
