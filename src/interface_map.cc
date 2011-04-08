#include "interface_map.h"

#include <string>
#include <map>

#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"

using std::string;


void InterfaceMap::interfaceIs(const Interface::PtrConst iface) {
  if (!iface)
    throw Fwk::ResourceException("InterfaceMap::interfaceIs",
                                 "Interface is NULL");

  name_if_map_[iface->name()] = iface;
  ip_if_map_[iface->ip()] = iface;
}


Interface::PtrConst InterfaceMap::interface(const string& name) const {
  NameInterfaceMap::const_iterator it = name_if_map_.find(name);
  return (it != name_if_map_.end()) ? it->second : NULL;
}


Interface::PtrConst InterfaceMap::interfaceAddr(const IPv4Addr& addr) const {
  IPInterfaceMap::const_iterator it = ip_if_map_.find(addr);
  return (it != ip_if_map_.end()) ? it->second : NULL;
}
