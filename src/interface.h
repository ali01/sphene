#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <string>

#include "fwk/named_interface.h"
#include "fwk/ptr.h"


class Interface : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const Interface> PtrConst;
  typedef Fwk::Ptr<Interface> Ptr;

  static Ptr InterfaceNew(const std::string& name) {
    return new Interface(name);
  }

 protected:
  Interface(const std::string& name);

 private:
  Interface(const Interface&);
  void operator=(const Interface&);
};

#endif
