#include "gtest/gtest.h"

#include <cstdlib>
#include <time.h>

#include "fwk/buffer.h"
using Fwk::Buffer;

#define BUFFER_SIZE 64

class BufferTest : public ::testing::Test {
 protected:
  BufferTest() {
    srand(time(NULL));
    for (int i = 0; i < BUFFER_SIZE; ++i)
      data_[i] = rand() % 256;
  }

  char data_[BUFFER_SIZE];
};

TEST_F(BufferTest, Construct) {
  Buffer::Ptr buffer = Buffer::BufferNew(data_, sizeof(data_));
  int cmp_val = memcmp(buffer->data(), data_, sizeof(data_));
  ASSERT_EQ(cmp_val, 0);
}
