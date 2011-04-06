#include "interface_map.h"

#include <string>
#include <map>

#include "fwk/exception.h"
#include "interface.h"

using std::string;


void InterfaceMap::interfaceIs(const Interface::PtrConst iface) {
  if (!iface)
    throw Fwk::ResourceException("InterfaceMap::interfaceIs",
                                 "Interface is NULL");

  map_[iface->name()] = iface;
}


Interface::PtrConst InterfaceMap::interface(const string& name) const {
  std::map<string, Interface::PtrConst>::const_iterator it = map_.find(name);
  return (it != map_.end()) ? it->second : NULL;
}
