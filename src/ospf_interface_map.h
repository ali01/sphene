#ifndef OSPF_NEIGHBOR_MAP_H_596RSZ34
#define OSPF_NEIGHBOR_MAP_H_596RSZ34

#include "fwk/linked_list.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_interface.h"
#include "ospf_types.h"


class OSPFInterfaceMap : public Fwk::PtrInterface<OSPFInterfaceMap> {
 public:
  typedef Fwk::Ptr<const OSPFInterfaceMap> PtrConst;
  typedef Fwk::Ptr<OSPFInterfaceMap> Ptr;

  typedef Fwk::Map<IPv4Addr,OSPFInterface>::iterator if_iter;
  typedef Fwk::Map<IPv4Addr,OSPFInterface>::const_iterator const_if_iter;

  typedef Fwk::Map<RouterID,OSPFGateway>::iterator gw_iter;
  typedef Fwk::Map<RouterID,OSPFGateway>::const_iterator const_gw_iter;

  static Ptr New() {
    return new OSPFInterfaceMap();
  }

  /* Accessors. */

  OSPFInterface::PtrConst interface(const IPv4Addr& addr) const;
  OSPFInterface::Ptr interface(const IPv4Addr& addr);

  OSPFInterface::PtrConst interface(const RouterID& neighbor_id) const;
  OSPFInterface::Ptr interface(const RouterID& neighbor_id);

  OSPFGateway::PtrConst gateway(const RouterID& id) const;
  OSPFGateway::Ptr gateway(const RouterID& id);

  /* Mutators. */

  void interfaceIs(OSPFInterface::Ptr iface_desc);
  void interfaceDel(OSPFInterface::Ptr iface_desc);
  void interfaceDel(const IPv4Addr& addr);

  /* Iterators. */

  if_iter ifacesBegin() { return ip_ifaces_.begin(); }
  if_iter ifacesEnd() { return ip_ifaces_.end(); }
  const_if_iter ifacesBegin() const { return ip_ifaces_.begin(); }
  const_if_iter ifacesEnd() const { return ip_ifaces_.end(); }

  gw_iter gatewaysBegin() { return gateways_.begin(); }
  gw_iter gatewaysEnd() { return gateways_.end(); }
  const_gw_iter gatewaysBegin() const { return gateways_.begin(); }
  const_gw_iter gatewaysEnd() const { return gateways_.end(); }

 protected:
  OSPFInterfaceMap();

 private:
  class InterfaceReactor : public OSPFInterface::Notifiee {
   public:
    typedef Fwk::Ptr<const InterfaceReactor> PtrConst;
    typedef Fwk::Ptr<InterfaceReactor> Ptr;

    static Ptr New(OSPFInterfaceMap::Ptr _im) {
      return new InterfaceReactor(_im);
    }

    void onGateway(OSPFInterface::Ptr iface, const RouterID& id);
    void onGatewayDel(OSPFInterface::Ptr iface, const RouterID& id);

   private:
    InterfaceReactor(OSPFInterfaceMap::Ptr _im) : iface_map_(_im.ptr()) {}

    /* Data members. */
    OSPFInterfaceMap* iface_map_; /* Weak ptr prevents circular reference. */

    /* Operations disallowed. */
    InterfaceReactor(const InterfaceReactor&);
    void operator=(const InterfaceReactor&);
  };

  /* Data members. */

  /* Map: Interface IP address => interface object. */
  Fwk::Map<IPv4Addr,OSPFInterface> ip_ifaces_;

  /* Map: RouterID => Interface object. */
  Fwk::Map<RouterID,OSPFInterface> nbr_ifaces_;

  /* Map: RouterID => Gateway object. */
  Fwk::Map<RouterID,OSPFGateway> gateways_;

  /* Reactor to Interface notifications. */
  InterfaceReactor::Ptr iface_reactor_;

  /* Friends */
  friend class InterfaceReactor;

  /* Operations disallowed. */
  OSPFInterfaceMap(const OSPFInterfaceMap&);
  void operator=(const OSPFInterfaceMap&);
};

#endif
