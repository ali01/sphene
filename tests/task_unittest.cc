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
    task1_ = BasicTask::New("task1");
    task2_ = BasicTask::New("task2");
    TimeEpoch now(time(NULL));
    task1_->timeIs(now);
    task2_->timeIs(now);
    task1_->periodIs(10);
    task2_->periodIs(20);
  }

  TaskManager::Ptr tm_;
  BasicTask::Ptr task1_;
  BasicTask::Ptr task2_;
};


TEST_F(TaskManagerTest, addTask) {
  ASSERT_EQ((size_t)0, tm_->tasks());

  // Add one task.
  tm_->taskIs(task1_);
  EXPECT_EQ((size_t)1, tm_->tasks());
  EXPECT_EQ(task1_.ptr(), tm_->task(task1_->name()).ptr());

  // Add second task.
  tm_->taskIs(task2_);
  EXPECT_EQ((size_t)2, tm_->tasks());
  EXPECT_EQ(task2_.ptr(), tm_->task(task2_->name()).ptr());

  // Test idempotency.
  tm_->taskIs(task2_);
  EXPECT_EQ((size_t)2, tm_->tasks());
};


TEST_F(TaskManagerTest, delTask) {
  // Add both tasks.
  tm_->taskIs(task1_);
  tm_->taskIs(task2_);
  EXPECT_EQ((size_t)2, tm_->tasks());

  // Delete task 1.
  tm_->taskDel(task1_);
  EXPECT_EQ((size_t)1, tm_->tasks());
  EXPECT_EQ(Task::Ptr(NULL), tm_->task(task1_->name()).ptr());
  EXPECT_EQ(task2_.ptr(), tm_->task(task2_->name()).ptr());
}


TEST_F(TaskManagerTest, delTaskByName) {
  // Add both tasks.
  tm_->taskIs(task1_);
  tm_->taskIs(task2_);
  EXPECT_EQ((size_t)2, tm_->tasks());

  // Delete task 1.
  tm_->taskDel(task1_->name());
  EXPECT_EQ((size_t)1, tm_->tasks());
  EXPECT_EQ(Task::Ptr(NULL), tm_->task(task1_->name()).ptr());
  EXPECT_EQ(task2_.ptr(), tm_->task(task2_->name()).ptr());
}


TEST_F(TaskManagerTest, timeIs) {
  TimeEpoch start(time(NULL));

  // Add both tasks.
  tm_->taskIs(task1_);
  tm_->taskIs(task2_);

  // Ensure both tasks have not run yet.
  ASSERT_EQ(0, task1_->value());
  ASSERT_EQ(0, task2_->value());

  // Advance clock for both tasks.
  Seconds large_interval = task1_->period().value() + task2_->period().value();
  tm_->timeIs(start.value() + large_interval.value());

  // Both tasks should have run once.
  ASSERT_EQ(1, task1_->value());
  ASSERT_EQ(1, task2_->value());

  // Advance the clock by the smaller of the two task intervals.
  tm_->timeIs(task1_->time().value() + task1_->period().value());

  // Only one task should have run.
  ASSERT_EQ(2, task1_->value());
  ASSERT_EQ(1, task2_->value());
}
