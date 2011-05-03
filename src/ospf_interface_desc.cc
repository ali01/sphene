#include "ospf_interface_desc.h"

#include "interface.h"

OSPFInterfaceDesc::Ptr
OSPFInterfaceDesc::New(Fwk::Ptr<const Interface> iface, uint16_t helloint) {
  return new OSPFInterfaceDesc(iface, helloint);
}

OSPFInterfaceDesc::OSPFInterfaceDesc(Interface::PtrConst iface,
                                     uint16_t helloint)
    : iface_(iface), helloint_(helloint) {}

Interface::PtrConst
OSPFInterfaceDesc::interface() const {
  return iface_;
}

OSPFNode::Ptr
OSPFInterfaceDesc::neighbor(const RouterID& router_id) {
  return neighbors_.elem(router_id);
}

OSPFNode::PtrConst
OSPFInterfaceDesc::neighbor(const RouterID& router_id) const {
  OSPFInterfaceDesc* self = const_cast<OSPFInterfaceDesc*>(this);
  return self->neighbor(router_id);
}

void
OSPFInterfaceDesc::neighborIs(OSPFNode::Ptr nb) {
  if (nb == NULL)
    return;

  neighbors_[nb->routerID()] = nb;
}

void
OSPFInterfaceDesc::neighborDel(OSPFNode::Ptr nb) {
  if (nb == NULL)
    return;

  this->neighborDel(nb->routerID());
}

void
OSPFInterfaceDesc::neighborDel(const RouterID& router_id) {
  neighbors_.elemDel(router_id);
}
