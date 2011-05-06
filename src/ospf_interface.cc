#include "ospf_interface.h"

#include "interface.h"

OSPFInterface::Ptr
OSPFInterface::New(Fwk::Ptr<const Interface> iface, uint16_t helloint) {
  return new OSPFInterface(iface, helloint);
}

OSPFInterface::OSPFInterface(Interface::PtrConst iface,
                             uint16_t helloint)
    : iface_(iface), helloint_(helloint) {}

Interface::PtrConst
OSPFInterface::interface() const {
  return iface_;
}

OSPFNode::Ptr
OSPFInterface::neighbor(const RouterID& router_id) {
  return neighbor_nodes_.elem(router_id);
}

OSPFNode::PtrConst
OSPFInterface::neighbor(const RouterID& router_id) const {
  OSPFInterface* self = const_cast<OSPFInterface*>(this);
  return self->neighbor(router_id);
}

IPv4Addr
OSPFInterface::neighborGateway(const RouterID& router_id) const {
  OSPFGateway::PtrConst nbr = neighbors_.elem(router_id);
  if (nbr)
    return nbr->gateway();

  return IPv4Addr::kZero;
}

IPv4Addr
OSPFInterface::neighborSubnet(const RouterID& router_id) const {
  OSPFGateway::PtrConst nbr = neighbors_.elem(router_id);
  if (nbr)
    return nbr->subnet();

  return IPv4Addr::kZero;
}

IPv4Addr
OSPFInterface::neighborSubnetMask(const RouterID& router_id) const {
  OSPFGateway::PtrConst nbr = neighbors_.elem(router_id);
  if (nbr)
    return nbr->subnetMask();

  return IPv4Addr::kMax;
}

void
OSPFInterface::neighborIs(OSPFNode::Ptr nb,
                          const IPv4Addr& gateway,
                          const IPv4Addr& subnet,
                          const IPv4Addr& subnet_mask) {
  if (nb == NULL)
    return;

  RouterID nd_id = nb->routerID();
  OSPFGateway::Ptr nbr_prev = neighbors_.elem(nd_id);
  if (nbr_prev->node() != nb
      || nbr_prev->subnet() != subnet
      || nbr_prev->subnetMask() != subnet_mask) {

    /* Adding to direct OSPFNode pointer map. */
    neighbor_nodes_[nd_id] = nb;

    /* Adding to OSPFGateway pointer map. */
    OSPFGateway::Ptr ospf_nbr =
      OSPFGateway::New(nb, gateway, subnet, subnet_mask);
    neighbors_[nd_id] = ospf_nbr;

    /* Signaling notifiee. */
    notifiee_->onNeighbor(this, nd_id);
  }
}

void
OSPFInterface::neighborDel(OSPFNode::Ptr nb) {
  if (nb == NULL)
    return;

  neighborDel(nb->routerID());
}

void
OSPFInterface::neighborDel(const RouterID& router_id) {
  /* Deleting from both maps. */
  neighbors_.elemDel(router_id);
  neighbor_nodes_.elemDel(router_id);

  /* Signaling notifiee. */
  notifiee_->onNeighborDel(this, router_id);
}
