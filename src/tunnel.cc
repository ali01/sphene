#include "tunnel.h"

#include <string>

#include "fwk/named_interface.h"
#include "interface.h"

using std::string;


Tunnel::Tunnel(Interface::PtrConst iface)
    : Fwk::NamedInterface(iface->name()),
      iface_(iface),
      remote_("0.0.0.0"),
      mode_(kGRE) { }
