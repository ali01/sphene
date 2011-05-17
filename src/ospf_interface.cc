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
OSPFInterface::interfaceSubnet() const {
  return iface_->subnet();
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

void
OSPFInterface::timeSinceOutgoingHelloIs(Seconds delta) {
  last_outgoing_hello_ = time(NULL) - delta.value();
}

OSPFGateway::Ptr
OSPFInterface::activeGateway(const RouterID& router_id) {
  return active_gateways_.elem(router_id);
}

OSPFGateway::PtrConst
OSPFInterface::activeGateway(const RouterID& router_id) const {
  return active_gateways_.elem(router_id);
}

OSPFGateway::Ptr
OSPFInterface::passiveGateway(const IPv4Addr& subnet, const IPv4Addr& mask) {
  return passive_gateways_.elem(std::make_pair(subnet, mask));
}

OSPFGateway::PtrConst
OSPFInterface::passiveGateway(const IPv4Addr& subnet,
                              const IPv4Addr& mask) const {
  return passive_gateways_.elem(std::make_pair(subnet, mask));
}

size_t
OSPFInterface::gateways() const {
  return activeGateways() + passiveGateways();
}

void
OSPFInterface::gatewayIs(OSPFGateway::Ptr gw_obj) {
  if (gw_obj == NULL)
    return;

  OSPFGateway::Ptr gw_prev;
  if (gw_obj->nodeIsPassiveEndpoint()) {
    IPv4Subnet subnet = std::make_pair(gw_obj->subnet(), gw_obj->subnetMask());
    gw_prev = passiveGateway(gw_obj->subnet(), gw_obj->subnetMask());
    passive_gateways_[subnet] = gw_obj;

  } else {
    RouterID nd_id = gw_obj->nodeRouterID();
    gw_prev = activeGateway(nd_id);
    active_gateways_[nd_id] = gw_obj;
  }

  /* Adding this interface to gw_obj. */
  gw_obj->interfaceIs(this);

  if (notifiee_ && (gw_prev == NULL || *gw_obj != *gw_prev))
    notifiee_->onGateway(this, gw_obj);
}

void
OSPFInterface::activeGatewayDel(const RouterID& router_id) {
  OSPFGateway::Ptr gw_obj = activeGateway(router_id);
  if (gw_obj) {
    active_gateways_.elemDel(router_id);

    /* Signaling notifiee. */
    if (notifiee_)
      notifiee_->onGatewayDel(this, gw_obj);
  }
}

void
OSPFInterface::passiveGatewayDel(const IPv4Addr& subnet, const IPv4Addr& mask) {
  OSPFGateway::Ptr gw_obj = passiveGateway(subnet, mask);
  if (gw_obj) {
    IPv4Subnet subnet_key = std::make_pair(subnet, mask);
    passive_gateways_.elemDel(subnet_key);

    /* Signaling notifiee. */
    if (notifiee_)
      notifiee_->onGatewayDel(this, gw_obj);
  }
}
