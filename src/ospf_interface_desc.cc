#include "ospf_interface_desc.h"

OSPFInterfaceDesc::OSPFInterfaceDesc(Interface::Ptr iface, uint16_t helloint)
    : iface_(iface), helloint_(helloint) {}

OSPFNeighbor::PtrConst
OSPFInterfaceDesc::neighbor(uint32_t router_id) const {
  OSPFNeighbor::PtrConst neighbor = NULL;
  const_iterator it = neighbors_.find(router_id);
  if (it != this->end())
    neighbor = it->second;

  return neighbor;
}

void
OSPFInterfaceDesc::neighborIs(OSPFNeighbor::PtrConst nb) {
  if (nb) {
    uint32_t key = nb->routerId();
    neighbors_[key] = nb;
  }
}

void
OSPFInterfaceDesc::neighborDel(OSPFNeighbor::PtrConst nb) {
  if (nb == NULL)
    return;

  uint32_t key = nb->routerId();
  this->neighborDel(key);
}

void
OSPFInterfaceDesc::neighborDel(uint32_t router_id) {
  neighbors_.erase(router_id);
}