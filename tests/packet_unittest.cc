#include "gtest/gtest.h"

#include "fwk/buffer.h"

#include "ethernet_packet.h"


TEST(PacketTest, Construct) {
  const char *data = "this is my string";

  Buffer::Ptr buf = Buffer::BufferNew(data, sizeof(data));
  EthernetPacket pkt(buf, 0);

  ASSERT_EQ(buf, pkt.buffer());
}
