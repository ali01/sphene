#include <string>

#include "fwk/named_interface.h"
#include "interface_map.h"

using std::string;


InterfaceMap::InterfaceMap(const std::string& name)
    : Fwk::NamedInterface(name) {

}
