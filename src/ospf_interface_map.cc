#include "ospf_interface_map.h"

#include "interface.h"
#include "ospf_constants.h"

/* OSPFInterfaceMap */

OSPFInterfaceMap::OSPFInterfaceMap(InterfaceMap::Ptr iface_map)
    : ospf_iface_reactor_(OSPFInterfaceReactor::New(this)),
      iface_map_reactor_(InterfaceMapReactor::New(this)) {
  iface_map_reactor_->notifierIs(iface_map);
}

OSPFInterface::PtrConst
OSPFInterfaceMap::interface(const IPv4Addr& addr) const {
  return ifaces_.elem(addr);
}

OSPFInterface::Ptr
OSPFInterfaceMap::interface(const IPv4Addr& addr) {
  return ifaces_.elem(addr);
}

OSPFGateway::PtrConst
OSPFInterfaceMap::activeGateway(const RouterID& rid) const {
  OSPFInterfaceMap* self = const_cast<OSPFInterfaceMap*>(this);
  return self->activeGateway(rid);
}

OSPFGateway::Ptr
OSPFInterfaceMap::activeGateway(const RouterID& rid) {
  OSPFGateway::Ptr gw_obj = NULL;
  for (const_if_iter if_it = ifacesBegin(); if_it != ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    gw_obj = iface->activeGateway(rid);
    if (gw_obj)
      break;
  }

  return gw_obj;
}

size_t
OSPFInterfaceMap::gateways() const {
  size_t gateways = 0;
  for (const_if_iter if_it = ifacesBegin(); if_it != ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    gateways += iface->gateways();
  }

  return gateways;
}

size_t
OSPFInterfaceMap::activeGateways() const {
  size_t active_gateways = 0;
  for (const_if_iter if_it = ifacesBegin(); if_it != ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    active_gateways += iface->activeGateways();
  }

  return active_gateways;
}

void
OSPFInterfaceMap::interfaceIs(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  IPv4Addr key = iface->interfaceIP();
  OSPFInterface::Ptr iface_prev = interface(key);
  if (iface_prev == iface) {
    /* Test for pointer equivalence.
       Early return if IFACE interface already exists in this interface map. */
    return;
  }

  ifaces_[key] = iface;

  /* Set this interface map as IFACE's notifiee. */
  iface->notifieeIs(ospf_iface_reactor_);

  /* Signal our own notifiee. */
  if (notifiee_)
    notifiee_->onInterface(this, iface);

  for (OSPFInterface::const_gwa_iter it = iface->activeGatewaysBegin();
       it != iface->activeGatewaysEnd(); ++it) {
    OSPFGateway::Ptr gw_obj = it->second;
    if (notifiee_)
      notifiee_->onGateway(this, iface, gw_obj);
  }

  for (OSPFInterface::const_gwp_iter it = iface->passiveGatewaysBegin();
       it != iface->passiveGatewaysEnd(); ++it) {
    OSPFGateway::Ptr gw_obj = it->second;
    if (notifiee_)
      notifiee_->onGateway(this, iface, gw_obj);
  }
}

void
OSPFInterfaceMap::interfaceDel(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  IPv4Addr key = iface->interfaceIP();
  iface = this->interface(key);
  if (iface == NULL) {
    /* Early return if IFACE doesn't exist in this interface map. */
    return;
  }

  /* Remove gateways. This will trigger notifications to notifiee_. */
  for (OSPFInterface::const_gwa_iter it = iface->activeGatewaysBegin();
       it != iface->activeGatewaysEnd(); ++it) {
    OSPFGateway::Ptr gw_obj = it->second;
    iface->activeGatewayDel(gw_obj->nodeRouterID());
  }

  for (OSPFInterface::const_gwp_iter it = iface->passiveGatewaysBegin();
       it != iface->passiveGatewaysEnd(); ++it) {
    OSPFGateway::Ptr gw_obj = it->second;
    iface->passiveGatewayDel(gw_obj->subnet(), gw_obj->subnetMask());
  }

  /* Remove interface only after notifiees have
     been notified about the removal of each gateway. */
  ifaces_.elemDel(key);

  /* Remove this interface map as IFACE's notifiee. */
  iface->notifieeIs(NULL);

  if (notifiee_)
    notifiee_->onInterfaceDel(this, iface);
}

void
OSPFInterfaceMap::interfaceDel(const IPv4Addr& addr) {
  interfaceDel(interface(addr));
}


/* OSPFInterfaceMap::OSPFInterfaceReactor */

void
OSPFInterfaceMap::OSPFInterfaceReactor::onGateway(OSPFInterface::Ptr iface,
                                                  OSPFGateway::Ptr gw_obj) {
  if (iface_map_->notifiee_)
    iface_map_->notifiee_->onGateway(iface_map_, iface, gw_obj);
}

void
OSPFInterfaceMap::OSPFInterfaceReactor::onGatewayDel(OSPFInterface::Ptr iface,
                                                     OSPFGateway::Ptr gw_obj) {
  if (iface_map_->notifiee_)
    iface_map_->notifiee_->onGatewayDel(iface_map_, iface, gw_obj);
}

/* OSPFInterfaceMap::InterfaceMapReactor */

void
OSPFInterfaceMap::InterfaceMapReactor::onInterface(InterfaceMap::Ptr map,
                                                   Interface::Ptr iface) {
  OSPFInterface::Ptr ospf_iface =
    OSPFInterface::New(iface, OSPF::kDefaultHelloInterval);

  iface_map_->interfaceIs(ospf_iface);
}

void
OSPFInterfaceMap::InterfaceMapReactor::onInterfaceDel(InterfaceMap::Ptr map,
                                                      Interface::Ptr iface) {
  iface_map_->interfaceDel(iface->ip());
}

void
OSPFInterfaceMap::InterfaceMapReactor::onInterfaceEnabled(InterfaceMap::Ptr map,
                                                          Interface::Ptr _if) {
  if (_if->enabled())
    this->onInterface(map, _if);
  else
    this->onInterfaceDel(map, _if);
}
