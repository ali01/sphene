#ifndef INTERFACE_MAP_H_
#define INTERFACE_MAP_H_

#include <map>
#include <string>

#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"
#include "interface.h"
#include "ip_packet.h"


class InterfaceMap : public Fwk::PtrInterface<InterfaceMap> {
 public:
  typedef Fwk::Ptr<const InterfaceMap> PtrConst;
  typedef Fwk::Ptr<InterfaceMap> Ptr;

  static Ptr InterfaceMapNew() {
    return new InterfaceMap();
  }

  // Adds a new interface to the map. If an interface with the same name
  // already exists, it is replaced with 'iface'.
  void interfaceIs(Interface::Ptr iface);

  // Returns an interface by name 'name'. Returns NULL if no interface exists
  // by that name.
  Interface::Ptr interface(const std::string& name);

  // Returns an interface with IP address 'addr'. Returns NULL if no interface
  // exists with that address.
  Interface::Ptr interfaceAddr(const IPv4Addr& addr);

  // Returns the number of interfaces in the map.
  size_t interfaces() const { return name_if_map_.size(); }

 protected:
  InterfaceMap() { }

 private:
  typedef std::map<std::string, Interface::Ptr> NameInterfaceMap;
  typedef std::map<IPv4Addr, Interface::Ptr> IPInterfaceMap;

  NameInterfaceMap name_if_map_;
  IPInterfaceMap ip_if_map_;

  InterfaceMap(const InterfaceMap&);
  void operator=(const InterfaceMap&);
};

#endif
