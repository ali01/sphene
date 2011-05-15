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
  return rid_gateways_.elem(router_id);
}

OSPFGateway::PtrConst
OSPFInterface::gateway(const RouterID& router_id) const {
  return rid_gateways_.elem(router_id);
}

OSPFGateway::Ptr
OSPFInterface::gateway(const IPv4Addr& gateway) {
  return ip_gateways_.elem(gateway);
}

OSPFGateway::PtrConst
OSPFInterface::gateway(const IPv4Addr& gateway) const {
  return ip_gateways_.elem(gateway);
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
  OSPFGateway::Ptr gw_prev = rid_gateways_.elem(nd_id);

  /* Adding to OSPFGateway pointer maps. */
  rid_gateways_[nd_id] = gw_obj;
  ip_gateways_[nd_id] = gw_obj;
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
    /* Deleting from all three maps. */
    rid_gateways_.elemDel(router_id);
    ip_gateways_.elemDel(router_id);
    neighbors_.elemDel(router_id);

    /* Signaling notifiee. */
    if (notifiee_)
      notifiee_->onGatewayDel(this, router_id);
  }
}

void
OSPFInterface::gatewayDel(const IPv4Addr& addr) {
  OSPFGateway::Ptr gw_obj = ip_gateways_[addr];
  if (gw_obj)
    gatewayDel(gw_obj->nodeRouterID());
}
