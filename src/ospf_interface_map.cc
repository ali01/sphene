#include "ospf_interface_map.h"

#include "interface.h"

/* OSPFInterfaceMap */

OSPFInterfaceMap::OSPFInterfaceMap()
    : iface_reactor_(InterfaceReactor::New(this)) {}

OSPFInterface::PtrConst
OSPFInterfaceMap::interface(const IPv4Addr& addr) const {
  OSPFInterfaceMap* self = const_cast<OSPFInterfaceMap*>(this);
  return self->interface(addr);
}

OSPFInterface::Ptr
OSPFInterfaceMap::interface(const IPv4Addr& addr) {
  return ip_ifaces_.elem(addr);
}

OSPFInterface::PtrConst
OSPFInterfaceMap::interface(const RouterID& nb_id) const {
  return nbr_ifaces_.elem(nb_id);
}

OSPFInterface::Ptr
OSPFInterfaceMap::interface(const RouterID& nb_id) {
  return nbr_ifaces_.elem(nb_id);
}

OSPFGateway::PtrConst
OSPFInterfaceMap::gateway(const RouterID& id) const {
  return gateways_.elem(id);
}

OSPFGateway::Ptr
OSPFInterfaceMap::gateway(const RouterID& id) {
  return gateways_.elem(id);
}

void
OSPFInterfaceMap::interfaceIs(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  IPv4Addr key = iface->interfaceIP();
  ip_ifaces_[key] = iface;

  OSPFInterface::const_gw_iter it;
  for (it = iface->gatewaysBegin(); it != iface->gatewaysEnd(); ++it) {
    OSPFGateway::Ptr gateway = it->second;
    RouterID nd_id = gateway->node()->routerID();

    nbr_ifaces_[nd_id] = iface;
    gateways_[nd_id] = gateway;
  }
}

void
OSPFInterfaceMap::interfaceDel(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  IPv4Addr key = iface->interfaceIP();
  ip_ifaces_.elemDel(key);

  OSPFInterface::const_gw_iter it;
  for (it = iface->gatewaysBegin(); it != iface->gatewaysEnd(); ++it) {
    OSPFGateway::Ptr gateway = it->second;
    RouterID nd_id = gateway->node()->routerID();

    nbr_ifaces_.elemDel(nd_id);
    gateways_.elemDel(nd_id);
  }
}

void
OSPFInterfaceMap::interfaceDel(const IPv4Addr& addr) {
  interfaceDel(interface(addr));
}


/* OSPFInterfaceMap::InterfaceReactor */

void
OSPFInterfaceMap::InterfaceReactor::onGateway(OSPFInterface::Ptr iface,
                                              const RouterID& id) {
  iface_map_->nbr_ifaces_[id] = iface;
  iface_map_->gateways_[id] = iface->gateway(id);
}

void
OSPFInterfaceMap::InterfaceReactor::onGatewayDel(OSPFInterface::Ptr iface,
                                                 const RouterID& id) {
  iface_map_->nbr_ifaces_.elemDel(id);
  iface_map_->gateways_.elemDel(id);
}
