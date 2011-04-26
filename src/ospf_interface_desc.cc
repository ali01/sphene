#include "ospf_interface_desc.h"

OSPFInterfaceDesc::OSPFInterfaceDesc(Interface::PtrConst iface,
                                     uint16_t helloint)
    : iface_(iface), helloint_(helloint) {}

OSPFNeighbor::Ptr
OSPFInterfaceDesc::neighbor(uint32_t router_id) const {
  OSPFNeighbor::Ptr neighbor = NULL;
  const_iterator it = neighbors_.find(router_id);
  if (it != this->end())
    neighbor = it->second;

  return neighbor;
}

void
OSPFInterfaceDesc::neighborIs(OSPFNeighbor::Ptr nb) {
  if (nb) {
    uint32_t key = nb->routerId();
    neighbors_[key] = nb;
  }
}

void
OSPFInterfaceDesc::neighborDel(OSPFNeighbor::Ptr nb) {
  if (nb == NULL)
    return;

  uint32_t key = nb->routerId();
  this->neighborDel(key);
}

void
OSPFInterfaceDesc::neighborDel(uint32_t router_id) {
  neighbors_.erase(router_id);
}
