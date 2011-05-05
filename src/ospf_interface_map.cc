#include "ospf_interface_map.h"

#include "interface.h"

/* OSPFInterfaceMap */

OSPFInterfaceDesc::PtrConst
OSPFInterfaceMap::interface(const IPv4Addr& addr) const {
  OSPFInterfaceMap* self = const_cast<OSPFInterfaceMap*>(this);
  return self->interface(addr);
}

OSPFInterfaceDesc::Ptr
OSPFInterfaceMap::interface(const IPv4Addr& addr) {
  return interfaces_.elem(addr);
}

void
OSPFInterfaceMap::interfaceIs(OSPFInterfaceDesc::Ptr iface_desc) {
  if (iface_desc == NULL)
    return;

  IPv4Addr key = iface_desc->interface()->ip();
  interfaces_[key] = iface_desc;
}

void
OSPFInterfaceMap::interfaceDel(OSPFInterfaceDesc::Ptr iface_desc) {
  if (iface_desc == NULL)
    return;

  IPv4Addr key = iface_desc->interface()->ip();
  this->interfaceDel(key);
}

void
OSPFInterfaceMap::interfaceDel(const IPv4Addr& addr) {
  interfaces_.elemDel(addr);
}
