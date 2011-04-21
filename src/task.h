#ifndef TASK_H_
#define TASK_H_

#include <string>

#include "fwk/named_interface.h"
#include "time_types.h"


class Task : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const Task> PtrConst;
  typedef Fwk::Ptr<Task> Ptr;

  static Ptr New(const std::string& name) {
    return new Task(name);
  }

 protected:
  Task(const std::string& name);
  virtual ~Task() { }
};

#endif
