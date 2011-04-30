#ifndef INTERFACE_MAP_H_
#define INTERFACE_MAP_H_

#include <map>
#include <string>

#include "fwk/locked_interface.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"
#include "interface.h"
#include "ip_packet.h"


class InterfaceMap : public Fwk::PtrInterface<InterfaceMap>,
                     public Fwk::LockedInterface {
 public:
  typedef Fwk::Ptr<const InterfaceMap> PtrConst;
  typedef Fwk::Ptr<InterfaceMap> Ptr;
  typedef std::map<std::string, Interface::Ptr> NameInterfaceMap;
  typedef std::map<IPv4Addr, Interface::Ptr> IPInterfaceMap;
  typedef NameInterfaceMap::iterator iterator;
  typedef NameInterfaceMap::const_iterator const_iterator;

  static Ptr InterfaceMapNew() {
    return new InterfaceMap();
  }

  // Adds a new interface to the map. If an interface with the same name
  // already exists, it is replaced with 'iface'.
  void interfaceIs(Interface::Ptr iface);

  // Remove an interface from the map.
  void interfaceDel(const std::string& name);
  void interfaceDel(Interface::Ptr iface);

  // Returns an interface by name 'name'. Returns NULL if no interface exists
  // by that name.
  Interface::Ptr interface(const std::string& name);

  // Returns an interface with IP address 'addr'. Returns NULL if no interface
  // exists with that address.
  Interface::Ptr interfaceAddr(const IPv4Addr& addr);

  // Returns the number of interfaces in the map.
  size_t interfaces() const { return name_if_map_.size(); }

  iterator begin() { return name_if_map_.begin(); }
  iterator end() { return name_if_map_.end(); }
  const_iterator begin() const { return name_if_map_.begin(); }
  const_iterator end() const { return name_if_map_.end(); }

 protected:
  InterfaceMap() { }

 private:
  NameInterfaceMap name_if_map_;
  IPInterfaceMap ip_if_map_;

  InterfaceMap(const InterfaceMap&);
  void operator=(const InterfaceMap&);
};

#endif
