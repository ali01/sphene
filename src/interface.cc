#include "interface.h"

#include <string>

#include "fwk/named_interface.h"

using std::string;


Interface::Interface(const std::string& name)
    : Fwk::NamedInterface(name) {

}
