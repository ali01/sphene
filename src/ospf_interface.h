#ifndef OSPF_INTERFACE_DESC_H_6WPUE39N
#define OSPF_INTERFACE_DESC_H_6WPUE39N

#include <ctime>
#include <string>

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ipv4_subnet.h"
#include "ospf_gateway.h"
#include "ospf_node.h"
#include "ospf_types.h"
#include "time_types.h"

/* Forward declarations. */
class Interface;


class OSPFInterface : public Fwk::PtrInterface<OSPFInterface> {
 public:
  typedef Fwk::Ptr<const OSPFInterface> PtrConst;
  typedef Fwk::Ptr<OSPFInterface> Ptr;

  /* Active gateway iterators. */
  typedef Fwk::Map<RouterID,OSPFGateway>::iterator gwa_iter;
  typedef Fwk::Map<RouterID,OSPFGateway>::const_iterator const_gwa_iter;

  /* Passive gateway iterators. */
  typedef Fwk::Map<IPv4Subnet,OSPFGateway>::iterator gwp_iter;
  typedef Fwk::Map<IPv4Subnet,OSPFGateway>::const_iterator const_gwp_iter;

  /* Factory constructor. */
  static Ptr New(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  /* Notification support. */
  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported. */
    virtual void onGateway(OSPFInterface::Ptr iface, OSPFGateway::Ptr gw) {}
    virtual void onGatewayDel(OSPFInterface::Ptr iface, OSPFGateway::Ptr gw) {}

   protected:
    Notifiee() {}
    virtual ~Notifiee() {}

   private:
    /* Operations disallowed. */
    Notifiee(const Notifiee&);
    void operator=(const Notifiee&);
  };

  /* Accessors. */

  Fwk::Ptr<const Interface> interface() const;
  IPv4Addr interfaceIP() const;
  IPv4Addr interfaceSubnet() const;
  IPv4Addr interfaceSubnetMask() const;
  std::string interfaceName() const;

  uint16_t helloint() const { return helloint_; }
  Seconds timeSinceOutgoingHello() const;

  OSPFGateway::Ptr activeGateway(const RouterID& router_id);
  OSPFGateway::PtrConst activeGateway(const RouterID& router_id) const;

  OSPFGateway::Ptr passiveGateway(const IPv4Addr& subnet, const IPv4Addr& mask);
  OSPFGateway::PtrConst passiveGateway(const IPv4Addr& subnet,
                                       const IPv4Addr& mask) const;

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  size_t gateways() const;
  size_t activeGateways() const { return active_gateways_.size(); }
  size_t passiveGateways() const { return passive_gateways_.size(); }

  /* Mutators. */

  void timeSinceOutgoingHelloIs(Seconds delta);

  void gatewayIs(OSPFGateway::Ptr gateway);
  void activeGatewayDel(const RouterID& router_id);
  void passiveGatewayDel(const IPv4Addr& subnet, const IPv4Addr& mask);
  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Iterators. */

  gwa_iter activeGatewaysBegin() { return active_gateways_.begin(); }
  gwa_iter activeGatewaysEnd() { return active_gateways_.end(); }
  const_gwa_iter activeGatewaysEnd() const { return active_gateways_.end(); }
  const_gwa_iter activeGatewaysBegin() const {
    return active_gateways_.begin();
  }

  gwp_iter passiveGatewaysBegin() { return passive_gateways_.begin(); }
  gwp_iter passiveGatewaysEnd() { return passive_gateways_.end(); }
  const_gwp_iter passiveGatewaysEnd() const { return passive_gateways_.end(); }
  const_gwp_iter passiveGatewaysBegin() const {
    return passive_gateways_.begin();
  }

 private:
  OSPFInterface(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  /* Data members. */
  Fwk::Ptr<const Interface> iface_;
  uint16_t helloint_;
  time_t last_outgoing_hello_;

  /* Map of all neighbors directly attached to this router. */
  Fwk::Map<RouterID,OSPFGateway> active_gateways_;
  Fwk::Map<IPv4Subnet,OSPFGateway> passive_gateways_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Operations disallowed. */
  OSPFInterface(const OSPFInterface&);
  void operator=(const OSPFInterface&);
};

#endif
