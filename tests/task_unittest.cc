#include "gtest/gtest.h"

#include <string>
#include <time.h>

#include "task.h"
#include "time_types.h"

using std::string;


class BasicTask : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const BasicTask> PtrConst;
  typedef Fwk::Ptr<BasicTask> Ptr;

  static Ptr New(const string& name) { return new BasicTask(name); }

  int value() const { return value_; }

 protected:
  BasicTask(const string& name) : PeriodicTask(name), value_(0) { }

  void run() {
    value_++;
  }

  int value_;
};


class PeriodicTaskTest : public ::testing::Test {
 protected:
  void SetUp() {
    task_ = BasicTask::New("task1");
    TimeEpoch now(time(NULL));
    task_->timeIs(now);
    task_->periodIs(10);
  }

  BasicTask::Ptr task_;
};


TEST_F(PeriodicTaskTest, construct) {
  ASSERT_TRUE(task_);

  // Task should not have run yet.
  EXPECT_EQ(0, task_->value());
}


TEST_F(PeriodicTaskTest, stepOneInterval) {
  // Advance the clock by the period.
  TimeEpoch now(time(NULL) + task_->period().value());
  task_->timeIs(now);

  // Task should have run once.
  EXPECT_EQ(1, task_->value());
}


TEST_F(PeriodicTaskTest, stepTwoIntervals) {
  TimeEpoch start(time(NULL));

  // Advance the clock by the period.
  TimeEpoch now(start.value() + task_->period().value());
  task_->timeIs(now);
  EXPECT_EQ(1, task_->value());

  // Advance by half a period and ensure the task does not run.
  now = now.value() + task_->period().value() / 2;
  task_->timeIs(now);
  EXPECT_EQ(1, task_->value());

  // Advance to the second interval.
  now = start.value() + task_->period().value() * 2;
  task_->timeIs(now);
  EXPECT_EQ(2, task_->value());

  // timeIs should be idempotent.
  task_->timeIs(now);
  EXPECT_EQ(2, task_->value());
}


class TaskManagerTest : public ::testing::Test {
 protected:
  void SetUp() {
    tm_ = TaskManager::New();
    task_ = BasicTask::New("task1");
  }

  TaskManager::Ptr tm_;
  PeriodicTask::Ptr task_;
};


TEST_F(TaskManagerTest, addTask) {
  ASSERT_EQ((size_t)0, tm_->tasks());
  tm_->taskIs(task_);
  EXPECT_EQ((size_t)1, tm_->tasks());
  EXPECT_EQ(task_.ptr(), tm_->task(task_->name()).ptr());

  // Test idempotency.
  tm_->taskIs(task_);
  EXPECT_EQ((size_t)1, tm_->tasks());
};


TEST_F(TaskManagerTest, delTask) {
  tm_->taskIs(task_);
  EXPECT_EQ((size_t)1, tm_->tasks());

  tm_->taskDel(task_);
  EXPECT_EQ((size_t)0, tm_->tasks());
}


TEST_F(TaskManagerTest, delTaskByName) {
  tm_->taskIs(task_);
  EXPECT_EQ((size_t)1, tm_->tasks());

  tm_->taskDel(task_->name());
  EXPECT_EQ((size_t)0, tm_->tasks());
}
