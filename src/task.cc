#include "task.h"

#include "fwk/named_interface.h"
#include "time_types.h"

#include <string>

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
