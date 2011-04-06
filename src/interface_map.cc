#include "interface_map.h"

#include <string>

#include "fwk/named_interface.h"
#include "interface.h"

using std::string;


InterfaceMap::InterfaceMap(const std::string& name)
    : Fwk::NamedInterface(name) {

}


void InterfaceMap::interfaceIs(Interface::PtrConst iface) {

}


Interface::PtrConst InterfaceMap::interface(const std::string& name) const {
  return NULL;
}
