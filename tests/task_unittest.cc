#include "gtest/gtest.h"

#include <cstdlib>
#include <cstring>
#include <time.h>

#include "task.h"


class TaskTest : public ::testing::Test {
 protected:
  void SetUp() {
    task_ = Task::New("task1");
  }

  Task::Ptr task_;
};


TEST_F(TaskTest, Construct) {
  ASSERT_TRUE(task_);
}
