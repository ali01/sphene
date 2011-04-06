#ifndef INTERFACE_MAP_H_
#define INTERFACE_MAP_H_

#include <string>

#include "fwk/named_interface.h"
#include "fwk/ptr.h"
#include "interface.h"


class InterfaceMap : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const InterfaceMap> PtrConst;
  typedef Fwk::Ptr<InterfaceMap> Ptr;

  static Ptr InterfaceMapNew(const std::string& name) {
    return new InterfaceMap(name);
  }

  // Adds a new interface to the map. If an interface with the same name
  // already exists, it is replaced with 'iface'.
  void interfaceIs(Interface::PtrConst iface);

  // Retrieves an interface by name 'name'. Returns NULL if no interface exists
  // by that name.
  Interface::PtrConst interface(const std::string& name) const;

 protected:
  InterfaceMap(const std::string& name);

 private:
  InterfaceMap(const InterfaceMap&);
  void operator=(const InterfaceMap&);
};

#endif
