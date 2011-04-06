#ifndef INTERFACE_MAP_H_
#define INTERFACE_MAP_H_

#include <map>
#include <string>

#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"
#include "interface.h"


class InterfaceMap : public Fwk::PtrInterface<InterfaceMap> {
 public:
  typedef Fwk::Ptr<const InterfaceMap> PtrConst;
  typedef Fwk::Ptr<InterfaceMap> Ptr;

  static Ptr InterfaceMapNew() {
    return new InterfaceMap();
  }

  // Adds a new interface to the map. If an interface with the same name
  // already exists, it is replaced with 'iface'.
  void interfaceIs(Interface::PtrConst iface);

  // Returns an interface by name 'name'. Returns NULL if no interface exists
  // by that name.
  Interface::PtrConst interface(const std::string& name) const;

  // Returns the number of interfaces in the map.
  size_t interfaces() const { return map_.size(); }

 protected:
  InterfaceMap() { }

 private:
  std::map<std::string, Interface::PtrConst> map_;

  InterfaceMap(const InterfaceMap&);
  void operator=(const InterfaceMap&);
};

#endif
