#include "ospf_neighbor_map.h"

/* OSPFNeighborMap */

OSPFInterfaceDesc::PtrConst
OSPFNeighborMap::interfaceDesc(const IPv4Addr& addr) const {
  OSPFNeighborMap* self = const_cast<OSPFNeighborMap*>(this);
  return self->interfaceDesc(addr);
}

OSPFInterfaceDesc::Ptr
OSPFNeighborMap::interfaceDesc(const IPv4Addr& addr) {
  OSPFInterfaceDesc::Ptr if_desc = NULL;
  const_iterator it = interfaces_.find(addr);
  if (it != this->end())
    if_desc = it->second;

  return if_desc;
}

void
OSPFNeighborMap::interfaceDescIs(OSPFInterfaceDesc::Ptr iface_desc) {
  if (iface_desc) {
    IPv4Addr key = iface_desc->interface()->ip();
    interfaces_[key] = iface_desc;
  }
}

void
OSPFNeighborMap::interfaceDescDel(OSPFInterfaceDesc::Ptr iface_desc) {
  if (iface_desc == NULL)
    return;

  IPv4Addr key = iface_desc->interface()->ip();
  this->interfaceDescDel(key);
}

void
OSPFNeighborMap::interfaceDescDel(const IPv4Addr& addr) {
  interfaces_.erase(addr);
}

