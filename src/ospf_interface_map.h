#ifndef OSPF_NEIGHBOR_MAP_H_596RSZ34
#define OSPF_NEIGHBOR_MAP_H_596RSZ34

#include "fwk/linked_list.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "interface_map.h"
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

  static Ptr New(InterfaceMap::Ptr iface_map) {
    return new OSPFInterfaceMap(iface_map);
  }

  /* Notification support. */
  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported */
    virtual void onInterface(OSPFInterfaceMap::Ptr, const IPv4Addr&) {}
    virtual void onInterfaceDel(OSPFInterfaceMap::Ptr, const IPv4Addr&) {}
    virtual void onGateway(OSPFInterfaceMap::Ptr, OSPFInterface::Ptr iface,
                           const RouterID&) {}
    virtual void onGatewayDel(OSPFInterfaceMap::Ptr, OSPFInterface::Ptr iface,
                              const RouterID&) {}

   protected:
    Notifiee() {}
    virtual ~Notifiee() {}

   private:
    /* Operations disallowed. */
    Notifiee(const Notifiee&);
    void operator=(const Notifiee&);
  };

  /* Accessors. */

  OSPFInterface::PtrConst interface(const IPv4Addr& iface_ip) const;
  OSPFInterface::Ptr interface(const IPv4Addr& iface_ip);

  OSPFInterface::PtrConst interface(const RouterID& neighbor_id) const;
  OSPFInterface::Ptr interface(const RouterID& neighbor_id);

  OSPFGateway::PtrConst gateway(const RouterID& id) const;
  OSPFGateway::Ptr gateway(const RouterID& id);

  size_t interfaces() const { return ip_ifaces_.size(); }
  size_t gateways() const;

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  /* Mutators. */

  void interfaceIs(OSPFInterface::Ptr iface_desc);
  void interfaceDel(OSPFInterface::Ptr iface_desc);
  void interfaceDel(const IPv4Addr& addr);

  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

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
  OSPFInterfaceMap(InterfaceMap::Ptr iface_map);

 private:
  class OSPFInterfaceReactor : public OSPFInterface::Notifiee {
   public:
    typedef Fwk::Ptr<const OSPFInterfaceReactor> PtrConst;
    typedef Fwk::Ptr<OSPFInterfaceReactor> Ptr;

    static Ptr New(OSPFInterfaceMap* _im) {
      return new OSPFInterfaceReactor(_im);
    }

    void onGateway(OSPFInterface::Ptr iface, const RouterID& id);
    void onGatewayDel(OSPFInterface::Ptr iface, const RouterID& id);

   private:
    OSPFInterfaceReactor(OSPFInterfaceMap* _im) : iface_map_(_im) {}

    /* Data members. */
    OSPFInterfaceMap* iface_map_; /* Weak ptr prevents circular reference. */

    /* Operations disallowed. */
    OSPFInterfaceReactor(const OSPFInterfaceReactor&);
    void operator=(const OSPFInterfaceReactor&);
  };

  class InterfaceMapReactor : public InterfaceMap::Notifiee {
   public:
    typedef Fwk::Ptr<const InterfaceMapReactor> PtrConst;
    typedef Fwk::Ptr<InterfaceMapReactor> Ptr;

    static Ptr New(OSPFInterfaceMap* _im) {
      return new InterfaceMapReactor(_im);
    }

    void onInterface(InterfaceMap::Ptr map, Interface::Ptr iface);
    void onInterfaceDel(InterfaceMap::Ptr map, Interface::Ptr iface);

   private:
    InterfaceMapReactor(OSPFInterfaceMap* _im) : iface_map_(_im) {}

    /* Data members. */
    OSPFInterfaceMap* iface_map_;

    /* Operations disallowed. */
    InterfaceMapReactor(const InterfaceMapReactor&);
    void operator=(const InterfaceMapReactor&);
  };

  /* Data members. */

  /* Map: Interface IP address => interface object. */
  Fwk::Map<IPv4Addr,OSPFInterface> ip_ifaces_;

  /* Map: RouterID => Interface object. */
  Fwk::Map<RouterID,OSPFInterface> nbr_ifaces_;

  /* Map: RouterID => Gateway object. */
  Fwk::Map<RouterID,OSPFGateway> gateways_;

  /* Reactors to Interface notifications. */
  OSPFInterfaceReactor::Ptr ospf_iface_reactor_;
  InterfaceMapReactor::Ptr iface_map_reactor_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Friends */
  friend class OSPFInterfaceReactor;

  /* Operations disallowed. */
  OSPFInterfaceMap(const OSPFInterfaceMap&);
  void operator=(const OSPFInterfaceMap&);
};

#endif
