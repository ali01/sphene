#include "task.h"

#include "fwk/named_interface.h"

#include <string>

using std::string;


Task::Task(const string& name) : Fwk::NamedInterface(name) {

}
