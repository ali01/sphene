#include "ospf_interface.h"

#include "interface.h"

OSPFInterface::Ptr
OSPFInterface::New(Fwk::Ptr<const Interface> iface, uint16_t helloint) {
  return new OSPFInterface(iface, helloint);
}

OSPFInterface::OSPFInterface(Interface::PtrConst iface, uint16_t helloint)
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

std::string
OSPFInterface::interfaceName() const {
  return iface_->name();
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
OSPFInterface::gatewayIs(OSPFGateway::Ptr gw_obj) {
  if (gw_obj == NULL)
    return;

  OSPFNode::Ptr node = gw_obj->node();
  RouterID nd_id = node->routerID();
  OSPFGateway::Ptr gw_prev = gateways_.elem(nd_id);

  /* Adding to OSPFGateway pointer map. */
  gateways_[nd_id] = gw_obj;

  /* Adding to direct OSPFNode pointer map. */
  neighbors_[nd_id] = node;

  /* Adding this interface to gw_obj. */
  gw_obj->interfaceIs(this);

  if (notifiee_ && (gw_prev == NULL || *gw_obj != *gw_prev))
    notifiee_->onGateway(this, nd_id);
}

void
OSPFInterface::gatewayDel(const RouterID& router_id) {
  OSPFGateway::Ptr gw_obj = gateway(router_id);
  if (gw_obj) {
    /* Deleting from both maps. */
    gateways_.elemDel(router_id);
    neighbors_.elemDel(router_id);

    /* Signaling notifiee. */
    if (notifiee_)
      notifiee_->onGatewayDel(this, router_id);
  }
}
