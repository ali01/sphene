#ifndef TASK_H_
#define TASK_H_

#include <string>

#include "fwk/named_interface.h"
#include "time_types.h"


class Task : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const Task> PtrConst;
  typedef Fwk::Ptr<Task> Ptr;

  virtual void timeIs(const TimeEpoch& t) = 0;

 protected:
  Task(const std::string& name);
  virtual ~Task() { }
};


// PeriodicTasks are tasks that run at specified intervals. Derived classes
// must define the run() method that is called at most once every period()
// seconds.
//
// If period() is 0 (default), the task is disabled.
class PeriodicTask : public Task {
 public:
  typedef Fwk::Ptr<const PeriodicTask> PtrConst;
  typedef Fwk::Ptr<PeriodicTask> Ptr;

  TimeEpoch time() const { return time_; }
  virtual void timeIs(const TimeEpoch& t);

  Seconds period() const { return period_; }
  void periodIs(const Seconds& s) { period_ = s; }

 protected:
  PeriodicTask(const std::string& name);
  virtual ~PeriodicTask() { }

  // Called every period() seconds.
  virtual void run() = 0;

 private:
  Seconds period_;
  TimeEpoch time_;
};

#endif
