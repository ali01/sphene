#ifndef INTERFACE_MAP_H_
#define INTERFACE_MAP_H_

#include <string>

#include "fwk/named_interface.h"
#include "fwk/ptr.h"


class InterfaceMap : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const InterfaceMap> PtrConst;
  typedef Fwk::Ptr<InterfaceMap> Ptr;

  static Ptr InterfaceMapNew(const std::string& name) {
    return new InterfaceMap(name);
  }

 protected:
  InterfaceMap(const std::string& name);

 private:
  InterfaceMap(const InterfaceMap&);
  void operator=(const InterfaceMap&);
};

#endif
