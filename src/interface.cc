#include "interface.h"

#include <string>

#include "fwk/named_interface.h"

using std::string;


Interface::Interface(const string& name)
    : Fwk::NamedInterface(name),
      enabled_(true),
      speed_(0),
      type_(kHardware),
      socket_(-1) { }
