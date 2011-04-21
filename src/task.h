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

#endif
