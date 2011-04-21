#include "gtest/gtest.h"

#include <string>
#include <time.h>

#include "task.h"
#include "time_types.h"

using std::string;


class BasicTask : public Task {
 public:
  typedef Fwk::Ptr<const BasicTask> PtrConst;
  typedef Fwk::Ptr<BasicTask> Ptr;

  static Ptr New(const string& name) { return new BasicTask(name); }

  void timeIs(const TimeEpoch& t) { }

 protected:
  BasicTask(const string& name) : Task(name) { }
};


class TaskTest : public ::testing::Test {
 protected:
  void SetUp() {
    task_ = BasicTask::New("task1");
    TimeEpoch now(time(NULL));
    task_->timeIs(now);
  }

  Task::Ptr task_;
};


TEST_F(TaskTest, Construct) {
  ASSERT_TRUE(task_);
}
