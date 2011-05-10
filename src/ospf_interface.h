#ifndef OSPF_INTERFACE_DESC_H_6WPUE39N
#define OSPF_INTERFACE_DESC_H_6WPUE39N

#include <ctime>

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
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

  typedef Fwk::Map<RouterID,OSPFNode>::iterator nb_iter;
  typedef Fwk::Map<RouterID,OSPFNode>::const_iterator const_nb_iter;

  typedef Fwk::Map<RouterID,OSPFGateway>::iterator gw_iter;
  typedef Fwk::Map<RouterID,OSPFGateway>::const_iterator const_gw_iter;

  static Ptr New(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  /* Notification support. */
  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported. */
    virtual void onGateway(OSPFInterface::Ptr iface, const RouterID& id) {}
    virtual void onGatewayDel(OSPFInterface::Ptr iface, const RouterID& id) {}

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
  IPv4Addr interfaceSubnetMask() const;

  uint16_t helloint() const { return helloint_; }
  Seconds timeSinceOutgoingHello() const;

  OSPFGateway::Ptr gateway(const RouterID& router_id);
  OSPFGateway::PtrConst gateway(const RouterID& router_id) const;

  OSPFNode::Ptr neighbor(const RouterID& router_id);
  OSPFNode::PtrConst neighbor(const RouterID& router_id) const;

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  size_t gateways() const { return gateways_.size(); }

  /* Mutators. */

  void timeSinceOutgoingHelloIs(Seconds delta);

  void gatewayIs(OSPFGateway::Ptr gateway);
  void gatewayDel(const RouterID& router_id);
  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Iterators. */

  nb_iter neighborsBegin() { return neighbors_.begin(); }
  nb_iter neighborsEnd() { return neighbors_.end(); }
  const_nb_iter neighborsBegin() const { return neighbors_.begin(); }
  const_nb_iter neighborsEnd() const { return neighbors_.end(); }

  gw_iter gatewaysBegin() { return gateways_.begin(); }
  gw_iter gatewaysEnd() { return gateways_.end(); }
  const_gw_iter gatewaysBegin() const { return gateways_.begin(); }
  const_gw_iter gatewaysEnd() const { return gateways_.end(); }

 private:
  OSPFInterface(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  /* Data members. */
  Fwk::Ptr<const Interface> iface_;
  uint16_t helloint_;
  time_t last_outgoing_hello_;

  /* Map of all neighbors directly attached to this router. */
  Fwk::Map<RouterID,OSPFGateway> gateways_;

  /* Mirror map with direct pointers to neighboring OSPFNodes (rather than
     OSPFLink objects). Used to provide iterators. If space constraints
     become a problem, this can be optimized away by defining custom
     iterators. */
  Fwk::Map<RouterID,OSPFNode> neighbors_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Operations disallowed. */
  OSPFInterface(const OSPFInterface&);
  void operator=(const OSPFInterface&);
};

#endif
