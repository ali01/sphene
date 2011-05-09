#include "ospf_interface_map.h"

#include "interface.h"
#include "ospf_constants.h"

/* OSPFInterfaceMap */

OSPFInterfaceMap::OSPFInterfaceMap()
    : iface_reactor_(OSPFInterfaceReactor::New(this)) {}

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
  OSPFInterface::Ptr iface_prev = interface(key);
  if (iface_prev == iface) {
    /* Early return if IFACE interface already
       exists in this interface map. */
    return;
  }

  /* Set this interface map as IFACE's notifiee. */
  iface->notifieeIs(iface_reactor_);

  ip_ifaces_[key] = iface;

  if (notifiee_)
    notifiee_->onInterface(this, key);

  OSPFInterface::const_gw_iter it;
  for (it = iface->gatewaysBegin(); it != iface->gatewaysEnd(); ++it) {
    OSPFGateway::Ptr gateway = it->second;
    RouterID nd_id = gateway->node()->routerID();

    nbr_ifaces_[nd_id] = iface;
    gateways_[nd_id] = gateway;

    if (notifiee_)
      notifiee_->onGateway(this, nd_id);
  }
}

void
OSPFInterfaceMap::interfaceDel(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  /* Remove this interface map as IFACE's notifiee. */
  iface->notifieeIs(NULL);

  IPv4Addr key = iface->interfaceIP();
  ip_ifaces_.elemDel(key);

  if (notifiee_)
    notifiee_->onInterfaceDel(this, key);

  OSPFInterface::const_gw_iter it;
  for (it = iface->gatewaysBegin(); it != iface->gatewaysEnd(); ++it) {
    OSPFGateway::Ptr gateway = it->second;
    RouterID nd_id = gateway->node()->routerID();

    nbr_ifaces_.elemDel(nd_id);
    gateways_.elemDel(nd_id);

    if (notifiee_)
      notifiee_->onGatewayDel(this, nd_id);
  }
}

void
OSPFInterfaceMap::interfaceDel(const IPv4Addr& addr) {
  interfaceDel(interface(addr));
}


/* OSPFInterfaceMap::OSPFInterfaceReactor */

void
OSPFInterfaceMap::OSPFInterfaceReactor::onGateway(OSPFInterface::Ptr iface,
                                                  const RouterID& id) {
  iface_map_->nbr_ifaces_[id] = iface;
  iface_map_->gateways_[id] = iface->gateway(id);
}

void
OSPFInterfaceMap::OSPFInterfaceReactor::onGatewayDel(OSPFInterface::Ptr iface,
                                                     const RouterID& id) {
  iface_map_->nbr_ifaces_.elemDel(id);
  iface_map_->gateways_.elemDel(id);
}

void
OSPFInterfaceMap::InterfaceMapReactor::onInterface(InterfaceMap::Ptr map,
                                                   Interface::Ptr iface) {
  OSPFInterface::Ptr ospf_iface =
    OSPFInterface::New(iface, OSPF::kDefaultLinkStateInterval);

  iface_map_->interfaceIs(ospf_iface);
}

void
OSPFInterfaceMap::InterfaceMapReactor::onInterfaceDel(InterfaceMap::Ptr map,
                                                      Interface::Ptr iface) {
  iface_map_->interfaceDel(iface->ip());
}
