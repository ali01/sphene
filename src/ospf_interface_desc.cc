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
  return neighbor_nodes_.elem(router_id);
}

OSPFNode::PtrConst
OSPFInterfaceDesc::neighbor(const RouterID& router_id) const {
  OSPFInterfaceDesc* self = const_cast<OSPFInterfaceDesc*>(this);
  return self->neighbor(router_id);
}

IPv4Addr
OSPFInterfaceDesc::neighborSubnet(const RouterID& router_id) const {
  OSPFNeighbor::PtrConst nbr = neighbors_.elem(router_id);
  if (nbr)
    return nbr->subnet();

  return IPv4Addr::kZero;
}

IPv4Addr
OSPFInterfaceDesc::neighborSubnetMask(const RouterID& router_id) const {
  OSPFNeighbor::PtrConst nbr = neighbors_.elem(router_id);
  if (nbr)
    return nbr->subnetMask();

  return IPv4Addr::kMax;
}

void
OSPFInterfaceDesc::neighborIs(OSPFNode::Ptr nb,
                              const IPv4Addr& subnet,
                              const IPv4Addr& subnet_mask) {
  if (nb == NULL)
    return;

  RouterID nd_id = nb->routerID();

  /* Adding to direct OSPFNode pointer map. */
  neighbor_nodes_[nd_id] = nb;

  /* Adding to OSPFNeighbor pointer map. */
  OSPFNeighbor::Ptr ospf_nbr = OSPFNeighbor::New(nb, subnet, subnet_mask);
  neighbors_[nd_id] = ospf_nbr;
}

void
OSPFInterfaceDesc::neighborDel(OSPFNode::Ptr nb) {
  if (nb == NULL)
    return;

  neighborDel(nb->routerID());
}

void
OSPFInterfaceDesc::neighborDel(const RouterID& router_id) {
  /* Deleting from both maps. */
  neighbors_.elemDel(router_id);
  neighbor_nodes_.elemDel(router_id);
}
