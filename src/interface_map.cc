#include "interface_map.h"

#include <string>
#include <map>

#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"

using std::string;


void InterfaceMap::interfaceIs(const Interface::Ptr iface) {
  if (!iface)
    throw Fwk::ResourceException("InterfaceMap::interfaceIs",
                                 "Interface is NULL");
  if (name_if_map_.find(iface->name()) != name_if_map_.end())
    return;

  name_if_map_[iface->name()] = iface;
  ip_if_map_[iface->ip()] = iface;

  // Dispatch notification.
  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    Ptr::st_cast<Notifiee>(notifiees_[i])->onInterface(iface);
}


void InterfaceMap::interfaceDel(const std::string& name) {
  Interface::Ptr iface = interface(name);
  interfaceDel(iface);
}


void InterfaceMap::interfaceDel(const Interface::Ptr iface) {
  // TODO(ms): Throw an exception here instead?
  if (!iface)
    return;
  if (name_if_map_.find(iface->name()) == name_if_map_.end())
    return;

  name_if_map_.erase(iface->name());
  ip_if_map_.erase(iface->ip());

  // Dispatch notification.
  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    Ptr::st_cast<Notifiee>(notifiees_[i])->onInterfaceDel(iface);
}


Interface::Ptr InterfaceMap::interface(const string& name) {
  NameInterfaceMap::iterator it = name_if_map_.find(name);
  return (it != name_if_map_.end()) ? it->second : NULL;
}


Interface::Ptr InterfaceMap::interfaceAddr(const IPv4Addr& addr) {
  IPInterfaceMap::iterator it = ip_if_map_.find(addr);
  return (it != ip_if_map_.end()) ? it->second : NULL;
}
