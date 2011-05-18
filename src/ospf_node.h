#ifndef OSPF_NODE_H_VKYMXJVI
#define OSPF_NODE_H_VKYMXJVI

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ipv4_subnet.h"
#include "ospf_link.h"
#include "ospf_types.h"


class OSPFNode : public Fwk::PtrInterface<OSPFNode> {
 public:
  typedef Fwk::Ptr<const OSPFNode> PtrConst;
  typedef Fwk::Ptr<OSPFNode> Ptr;

  /* Iterators for links to active OSPF nodes. */
  typedef Fwk::Map<RouterID,OSPFLink>::iterator lna_iter;
  typedef Fwk::Map<RouterID,OSPFLink>::const_iterator const_lna_iter;

  /* Iterators for links to passive endpoints. */
  typedef Fwk::Map<IPv4Subnet,OSPFLink>::iterator lnp_iter;
  typedef Fwk::Map<IPv4Subnet,OSPFLink>::const_iterator const_lnp_iter;

  static const OSPFNode::Ptr kPassiveEndpoint;
  static const uint16_t kMaxDistance = 0xffff;

  /* Factory constructor. */
  static Ptr New(const RouterID& router_id) {
    return new OSPFNode(router_id);
  }

  /* Notification support. */
  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported. */
    virtual void onLink(OSPFNode::Ptr node, OSPFLink::Ptr link, bool commit) {}
    virtual void onLinkDel(OSPFNode::Ptr node, OSPFLink::Ptr link, bool) {}

   protected:
    Notifiee() {}
    virtual ~Notifiee() {}

   private:
    /* Operations disallowed. */
    Notifiee(const Notifiee&);
    void operator=(const Notifiee&);
  };

  /* Accessors. */ 

  const RouterID& routerID() const { return router_id_; }

  OSPFLink::Ptr activeLink(const RouterID& id);
  OSPFLink::PtrConst activeLink(const RouterID& id) const;

  OSPFLink::Ptr passiveLink(const IPv4Addr& subnet, const IPv4Addr mask);
  OSPFLink::PtrConst passiveLink(const IPv4Addr& subnet,
                                 const IPv4Addr mask) const;

  size_t links() const { return activeLinks() + passiveLinks(); }
  size_t activeLinks() const { return active_links_.size(); }
  size_t passiveLinks() const { return passive_links_.size(); }

  /* Previous node in the shortest path from the root node to this node. */
  OSPFNode::Ptr prev() { return prev_; }
  OSPFNode::PtrConst prev() const { return prev_; }

  uint16_t latestSeqno() const { return latest_seqno_; }
  uint16_t distance() const { return distance_; }

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  bool isPassiveEndpoint() const;

  /* Mutators. */
  void routerIDIs(const RouterID& id) { router_id_ = id; }

  void linkIs(OSPFLink::Ptr link, bool commit=true);
  void activeLinkDel(const RouterID& id, bool commit=true);
  void passiveLinkDel(const IPv4Addr& subnet, const IPv4Addr& mask,
                      bool commit=true);

  void prevIs(OSPFNode::Ptr prev) { prev_ = prev; }

  void latestSeqnoIs(uint16_t seqno) { latest_seqno_ = seqno; }
  void distanceIs(uint16_t dist) { distance_ = dist; }
  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Iterators. */

  lna_iter activeLinksBegin() { return active_links_.begin(); }
  lna_iter activeLinksEnd() { return active_links_.end(); }
  const_lna_iter activeLinksBegin() const { return active_links_.begin(); }
  const_lna_iter activeLinksEnd() const { return active_links_.end(); }

  lnp_iter passiveLinksBegin() { return passive_links_.begin(); }
  lnp_iter passiveLinksEnd() { return passive_links_.end(); }
  const_lnp_iter passiveLinksBegin() const { return passive_links_.begin(); }
  const_lnp_iter passiveLinksEnd() const { return passive_links_.end(); }

 protected:
  OSPFNode(const RouterID& router_id);

 private:
  /* Helper member functions. */
  void oneWayLinkIs(OSPFLink::Ptr link, bool commit);
  void oneWayActiveLinkDel(const RouterID& id, bool commit);
  void oneWayPassiveLinkDel(const IPv4Addr& subnet, const IPv4Addr& mask,
                            bool commit);

  /* Data members. */
  RouterID router_id_;
  uint16_t latest_seqno_;
  uint16_t distance_;

  /* Previous node in the shortest path from the root node to this node. */
  OSPFNode::Ptr prev_;

  /* Map of all neighbors directly attached to this node. */
  Fwk::Map<RouterID,OSPFLink> active_links_;
  Fwk::Map<IPv4Subnet,OSPFLink> passive_links_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Operations disallowed. */
  OSPFNode(const OSPFNode&);
  void operator=(const OSPFNode&);
};

#endif
