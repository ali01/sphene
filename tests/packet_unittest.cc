#include "gtest/gtest.h"

#include <cstring>

#include "ethernet_packet.h"
#include "packet_buffer.h"


TEST(PacketTest, Size) {
  const char *data = "01234567890123456789";  // 20 bytes

  PacketBuffer::Ptr buf = PacketBuffer::New(data, strlen(data));
  EthernetPacket::Ptr pkt =
      EthernetPacket::New(buf, buf->size() - strlen(data));

  ASSERT_EQ(buf, pkt->buffer());
  ASSERT_EQ(strlen(data), pkt->len());
}
