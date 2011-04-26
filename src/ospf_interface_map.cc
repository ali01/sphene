#include "ospf_interface_map.h"

/* OSPFInterfaceMap */

OSPFInterfaceDesc::PtrConst
OSPFInterfaceMap::interfaceDesc(const IPv4Addr& addr) const {
  OSPFInterfaceMap* self = const_cast<OSPFInterfaceMap*>(this);
  return self->interfaceDesc(addr);
}

OSPFInterfaceDesc::Ptr
OSPFInterfaceMap::interfaceDesc(const IPv4Addr& addr) {
  return interfaces_.elem(addr);
}

void
OSPFInterfaceMap::interfaceDescIs(OSPFInterfaceDesc::Ptr iface_desc) {
  if (iface_desc == NULL)
    return;

  IPv4Addr key = iface_desc->interface()->ip();
  interfaces_[key] = iface_desc;
}

void
OSPFInterfaceMap::interfaceDescDel(OSPFInterfaceDesc::Ptr iface_desc) {
  if (iface_desc == NULL)
    return;

  IPv4Addr key = iface_desc->interface()->ip();
  this->interfaceDescDel(key);
}

void
OSPFInterfaceMap::interfaceDescDel(const IPv4Addr& addr) {
  interfaces_.elemDel(addr);
}
