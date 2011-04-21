#include "task.h"

#include <map>
#include <string>

#include "fwk/exception.h"
#include "fwk/named_interface.h"
#include "time_types.h"

using std::map;
using std::string;


Task::Task(const string& name) : Fwk::NamedInterface(name) {

}


PeriodicTask::PeriodicTask(const string& name)
  : Task(name), period_(0), time_(0) {

}


void PeriodicTask::timeIs(const TimeEpoch& t) {
  Seconds delta = (Seconds)(t - time_);

  if (period_ > 0 && delta >= period_) {
    run();

    // Only advance to the time last run.
    time_ = t;
  }
}


Task::Ptr TaskManager::task(const std::string& name) const {
  TaskMap::const_iterator it = task_map_.find(name);
  return (it != task_map_.end()) ? it->second : NULL;
}


void TaskManager::taskIs(Task::Ptr task) {
  task_map_[task->name()] = task;
}


void TaskManager::taskDel(Task::Ptr task) {
  task_map_.erase(task->name());
}


void TaskManager::taskDel(const std::string& name) {
  task_map_.erase(name);
}
