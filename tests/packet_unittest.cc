#include "gtest/gtest.h"

#include "buffer.h"
#include "packet.h"


TEST(PacketTest, Construct) {
  char data[64];
  Buffer::Ptr buf = Buffer::BufferNew(data, sizeof(data));
  Packet pkt(buf);

  ASSERT_EQ(buf, pkt.buffer());
}
