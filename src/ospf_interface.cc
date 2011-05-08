#include "ospf_interface.h"

#include "interface.h"

OSPFInterface::Ptr
OSPFInterface::New(Fwk::Ptr<const Interface> iface, uint16_t helloint) {
  return new OSPFInterface(iface, helloint);
}

OSPFInterface::OSPFInterface(Interface::PtrConst iface,
                             uint16_t helloint)
    : iface_(iface), helloint_(helloint), last_outgoing_hello_(0) {}

Interface::PtrConst
OSPFInterface::interface() const {
  return iface_;
}

IPv4Addr
OSPFInterface::interfaceIP() const {
  return iface_->ip();
}

IPv4Addr
OSPFInterface::interfaceSubnetMask() const {
  return iface_->subnetMask();
}

Seconds
OSPFInterface::timeSinceOutgoingHello() const {
  return time(NULL) - last_outgoing_hello_;
}

OSPFGateway::Ptr
OSPFInterface::gateway(const RouterID& router_id) {
  return gateways_.elem(router_id);
}

OSPFGateway::PtrConst
OSPFInterface::gateway(const RouterID& router_id) const {
  return gateways_.elem(router_id);
}

OSPFNode::Ptr
OSPFInterface::neighbor(const RouterID& router_id) {
  return neighbors_.elem(router_id);
}

OSPFNode::PtrConst
OSPFInterface::neighbor(const RouterID& router_id) const {
  OSPFInterface* self = const_cast<OSPFInterface*>(this);
  return self->neighbor(router_id);
}

void
OSPFInterface::timeSinceOutgoingHelloIs(Seconds delta) {
  last_outgoing_hello_ = time(NULL) - delta.value();
}

void
OSPFInterface::gatewayIs(OSPFNode::Ptr nb,
                         const IPv4Addr& gateway,
                         const IPv4Addr& subnet,
                         const IPv4Addr& subnet_mask) {
  if (nb == NULL)
    return;

  RouterID nd_id = nb->routerID();
  OSPFGateway::Ptr nbr_prev = gateways_.elem(nd_id);
  if (nbr_prev->node() != nb
      || nbr_prev->subnet() != subnet
      || nbr_prev->subnetMask() != subnet_mask) {

    /* Adding to direct OSPFNode pointer map. */
    neighbors_[nd_id] = nb;

    /* Adding to OSPFGateway pointer map. */
    OSPFGateway::Ptr gw_obj =
      OSPFGateway::New(nb, gateway, subnet, subnet_mask);
    gateways_[nd_id] = gw_obj;

    /* Signaling notifiee. */
    notifiee_->onGateway(this, nd_id);
  }
}

void
OSPFInterface::gatewayDel(const RouterID& router_id) {
  /* Deleting from both maps. */
  gateways_.elemDel(router_id);
  neighbors_.elemDel(router_id);

  /* Signaling notifiee. */
  notifiee_->onGatewayDel(this, router_id);
}
