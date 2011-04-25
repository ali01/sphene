#include "ospf_neighbor_map.h"

/* OSPFNeighborMap */

OSPFNeighborMap::InterfaceDesc::PtrConst
OSPFNeighborMap::interfaceDesc(const IPv4Addr& addr) const {
  OSPFNeighborMap* self = const_cast<OSPFNeighborMap*>(this);
  return self->interfaceDesc(addr);
}

OSPFNeighborMap::InterfaceDesc::Ptr
OSPFNeighborMap::interfaceDesc(const IPv4Addr& addr) {
  InterfaceDesc::Ptr if_desc = NULL;
  const_iterator it = interfaces_.find(addr);
  if (it != this->end())
    if_desc = it->second;

  return if_desc;
}

void
OSPFNeighborMap::interfaceDescIs(InterfaceDesc::Ptr iface_desc) {
  if (iface_desc) {
    IPv4Addr key = iface_desc->interface()->ip();
    interfaces_[key] = iface_desc;
  }
}

void
OSPFNeighborMap::interfaceDescDel(InterfaceDesc::Ptr iface_desc) {
  if (iface_desc == NULL)
    return;

  IPv4Addr key = iface_desc->interface()->ip();
  this->interfaceDescDel(key);
}

void
OSPFNeighborMap::interfaceDescDel(const IPv4Addr& addr) {
  interfaces_.erase(addr);
}

/* OSPFNeighborMap::Neighbor */

OSPFNeighborMap::Neighbor::Neighbor(uint32_t id, IPv4Addr iface_addr)
    : id_(id), iface_addr_(iface_addr) {}


OSPFNeighborMap::InterfaceDesc::InterfaceDesc(Interface::Ptr iface,
                                              uint16_t helloint)
    : iface_(iface), helloint_(helloint) {}

