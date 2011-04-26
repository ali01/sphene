#include "gtest/gtest.h"

#include <cstdlib>
#include <cstring>
#include <inttypes.h>

#include "packet_buffer.h"


class PacketBufferTest : public ::testing::Test {
 protected:
  void SetUp() { }

  PacketBuffer::Ptr pbuf_;
};


TEST_F(PacketBufferTest, constructNew) {
  // Ensure buffer is constructed in sizes of powers of 2.
  pbuf_ = PacketBuffer::New(511);
  EXPECT_EQ((size_t)512, pbuf_->size());

  pbuf_ = PacketBuffer::New(512);
  EXPECT_EQ((size_t)512, pbuf_->size());

  pbuf_ = PacketBuffer::New(513);
  EXPECT_EQ((size_t)1024, pbuf_->size());
}


TEST_F(PacketBufferTest, constructExisting) {
  // Construct a PacketBuffer as a copy of an existing buffer.
  const char* const buf = "This is a buffer.";
  pbuf_ = PacketBuffer::New((uint8_t*)buf, strlen(buf) + 1);

  // The minimum size should be sufficiently large.
  ASSERT_GT(pbuf_->size(), strlen(buf) + 1);

  // The string should be copied to the END of the buffer.
  const char* const str_ptr =
      (const char*)(pbuf_->data() + pbuf_->len() - strlen(buf) - 1);
  EXPECT_EQ(0, strcmp(buf, str_ptr));
}
