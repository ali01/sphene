#include "ospf_interface_desc.h"

OSPFInterfaceDesc::OSPFInterfaceDesc(Interface::PtrConst iface,
                                     uint16_t helloint)
    : iface_(iface), helloint_(helloint) {}

OSPFNeighbor::Ptr
OSPFInterfaceDesc::neighbor(uint32_t router_id) {
  return neighbors_.elem(router_id);
}

OSPFNeighbor::PtrConst
OSPFInterfaceDesc::neighbor(uint32_t router_id) const {
  OSPFInterfaceDesc* self = const_cast<OSPFInterfaceDesc*>(this);
  return self->neighbor(router_id);
}

void
OSPFInterfaceDesc::neighborIs(OSPFNeighbor::Ptr nb) {
  if (nb == NULL)
    return;

  neighbors_[nb->routerId()] = nb;
}

void
OSPFInterfaceDesc::neighborDel(OSPFNeighbor::Ptr nb) {
  if (nb == NULL)
    return;

  this->neighborDel(nb->routerId());
}

void
OSPFInterfaceDesc::neighborDel(uint32_t router_id) {
  neighbors_.elemDel(router_id);
}
