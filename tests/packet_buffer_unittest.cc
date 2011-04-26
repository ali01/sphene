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


TEST_F(PacketBufferTest, growBuffer) {
  const char* const buf = "This is a buffer.";
  pbuf_ = PacketBuffer::New((uint8_t*)buf, strlen(buf) + 1);

  // Minimum size assertions should not grow the buffer if they are not larger
  // than the buffer's size.
  const uint8_t* const old_ptr = pbuf_->data();
  const size_t old_size = pbuf_->size();
  pbuf_->minimumSizeIs(0);
  pbuf_->minimumSizeIs(1);
  pbuf_->minimumSizeIs(strlen(buf) + 1);
  pbuf_->minimumSizeIs(old_size);
  EXPECT_EQ(old_ptr, pbuf_->data());

  // Assert that the minimum buffer size should be larger than it currently is.
  pbuf_->minimumSizeIs(old_size + 1);
  EXPECT_NE(old_ptr, pbuf_->data());
  EXPECT_GT(pbuf_->size(), old_size);

  // The buffer data should have been copied.
  // The string should be copied to the END of the buffer.
  const char* const str_ptr =
      (const char*)(pbuf_->data() + pbuf_->len() - strlen(buf) - 1);
  EXPECT_EQ(0, strcmp(buf, str_ptr));
}
