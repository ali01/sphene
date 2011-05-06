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
  return ip_ifaces.elem(addr);
}

OSPFInterface::PtrConst
OSPFInterfaceMap::interface(const RouterID& nb_id) const {
  return nbr_id_ifaces_.elem(nb_id);
}

OSPFInterface::Ptr
OSPFInterfaceMap::interface(const RouterID& nb_id) {
  return nbr_id_ifaces_.elem(nb_id);
}

void
OSPFInterfaceMap::interfaceIs(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  IPv4Addr key = iface->interface()->ip();
  ip_ifaces[key] = iface;

  OSPFInterface::const_iterator it;
  for (it = iface->neighborsBegin(); it != iface->neighborsEnd(); ++it) {
    OSPFNode::Ptr node = it->second;
    nbr_id_ifaces_[node->routerID()] = iface;
  }
}

void
OSPFInterfaceMap::interfaceDel(OSPFInterface::Ptr iface) {
  if (iface == NULL)
    return;

  IPv4Addr key = iface->interface()->ip();
  ip_ifaces.elemDel(key);

  OSPFInterface::const_iterator it;
  for (it = iface->neighborsBegin(); it != iface->neighborsEnd(); ++it) {
    OSPFNode::Ptr node = it->second;
    nbr_id_ifaces_.elemDel(node->routerID());
  }
}

void
OSPFInterfaceMap::interfaceDel(const IPv4Addr& addr) {
  interfaceDel(interface(addr));
}


/* OSPFInterfaceMap::InterfaceReactor */

void
OSPFInterfaceMap::InterfaceReactor::onNeighbor(OSPFInterface::Ptr iface,
                                               const RouterID& id) {
  iface_map_->nbr_id_ifaces_[id] = iface;
}

void
OSPFInterfaceMap::InterfaceReactor::onNeighborDel(OSPFInterface::Ptr iface,
                                                  const RouterID& id) {
  iface_map_->nbr_id_ifaces_.elemDel(id);
}
