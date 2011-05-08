#ifndef OSPF_NODE_H_VKYMXJVI
#define OSPF_NODE_H_VKYMXJVI

#include <ctime>

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_link.h"
#include "ospf_types.h"


class OSPFNode : public Fwk::PtrInterface<OSPFNode> {
 public:
  typedef Fwk::Ptr<const OSPFNode> PtrConst;
  typedef Fwk::Ptr<OSPFNode> Ptr;

  typedef Fwk::Map<RouterID,OSPFNode>::iterator nb_iter;
  typedef Fwk::Map<RouterID,OSPFNode>::const_iterator const_nb_iter;

  typedef Fwk::Map<RouterID,OSPFLink>::iterator link_iter;
  typedef Fwk::Map<RouterID,OSPFLink>::const_iterator const_link_iter;

  static const uint16_t kMaxDistance = 0xffff;

  static Ptr New(const RouterID& router_id) {
    return new OSPFNode(router_id);
  }

  /* Notification support. */
  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported. */
    virtual void onLink(const RouterID& id) {}
    virtual void onLinkDel(const RouterID& id) {}

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

  OSPFLink::Ptr link(const RouterID& id);
  OSPFLink::PtrConst link(const RouterID& id) const;

  OSPFNode::Ptr neighbor(const RouterID& id);
  OSPFNode::PtrConst neighbor(const RouterID& id) const;

  /* Previous node in the shortest path from the root node to this node. */
  OSPFNode::Ptr prev() { return prev_; }
  OSPFNode::PtrConst prev() const { return prev_; }

  time_t timeSinceLSU() const { return time(NULL) - last_lsu_; }
  uint16_t latestSeqno() const { return latest_seqno_; }
  uint16_t distance() const { return distance_; }

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  /* Mutators. */

  void linkIs(OSPFNode::Ptr node,
              const IPv4Addr& subnet,
              const IPv4Addr& subnet_mask);
  void linkDel(const RouterID& id);

  void prevIs(OSPFNode::Ptr prev) { prev_ = prev; }

  void timeSinceLSUIs(time_t _t)  { last_lsu_ = time(NULL) - _t; }
  void latestSeqnoIs(uint16_t seqno) { latest_seqno_ = seqno; }
  void distanceIs(uint16_t dist) { distance_ = dist; }
  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Iterators. */

  nb_iter neighborsBegin() { return neighbors_.begin(); }
  nb_iter neighborsEnd() { return neighbors_.end(); }
  const_nb_iter neighborsBegin() const { return neighbors_.begin(); }
  const_nb_iter neighborsEnd() const { return neighbors_.end(); }

  link_iter linksBegin() { return links_.begin(); }
  link_iter linksEnd() { return links_.end(); }
  const_link_iter linksBegin() const { return links_.begin(); }
  const_link_iter linksEnd() const { return links_.end(); }

 private:
  OSPFNode(const RouterID& router_id);

  /* Data members. */
  RouterID router_id_;
  time_t last_lsu_;
  uint16_t latest_seqno_;
  uint16_t distance_;

  /* Previous node in the shortest path from the root node to this node. */
  OSPFNode::Ptr prev_;

  /* Map of all neighbors directly attached to this node. */
  Fwk::Map<RouterID,OSPFLink> links_;

  /* Mirror map with direct pointers to neighboring OSPFNodes (rather than
     OSPFLink objects). Used to provide iterators. If space constraints
     become a problem, this can be optimized away by defining custom
     iterators. */
  Fwk::Map<RouterID,OSPFNode> neighbors_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Operations disallowed. */
  OSPFNode(const OSPFNode&);
  void operator=(const OSPFNode&);
};

#endif
