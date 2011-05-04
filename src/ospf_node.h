#ifndef OSPF_NODE_H_VKYMXJVI
#define OSPF_NODE_H_VKYMXJVI

#include <ctime>

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_neighbor.h"
#include "ospf_types.h"


class OSPFNode : public Fwk::PtrInterface<OSPFNode> {
 public:
  typedef Fwk::Ptr<const OSPFNode> PtrConst;
  typedef Fwk::Ptr<OSPFNode> Ptr;

  typedef Fwk::Map<RouterID,OSPFNode>::iterator iterator;
  typedef Fwk::Map<RouterID,OSPFNode>::const_iterator const_iterator;

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
    virtual void onNeighbor(const RouterID& id) {}
    virtual void onNeighborDel(const RouterID& id) {}

   protected:
    Notifiee() {}

   private:
    /* Operations disallowed. */
    Notifiee(const Notifiee&);
    void operator=(const Notifiee&);
  };

  /* Accessors. */ 

  const RouterID& routerID() const { return router_id_; }

  OSPFNode::Ptr neighbor(const RouterID& id);
  OSPFNode::PtrConst neighbor(const RouterID& id) const;

  IPv4Addr neighborSubnet(const RouterID& neighbor_id) const;
  IPv4Addr neighborSubnetMask(const RouterID& neighbor_id) const;

  /* Previous node in the shortest path from the root node to this node. */
  OSPFNode::Ptr prev() { return prev_; }
  OSPFNode::PtrConst prev() const { return prev_; }

  time_t age() const { return time(NULL) - last_refreshed_; }
  uint16_t latestSeqno() const { return latest_seqno_; }
  uint16_t distance() const { return distance_; }

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  /* Mutators. */

  void neighborIs(OSPFNode::Ptr node,
                  const IPv4Addr& subnet,
                  const IPv4Addr& subnet_mask);
  void neighborDel(const RouterID& id);
  void neighborDel(OSPFNode::Ptr node);

  void prevIs(OSPFNode::Ptr prev) { prev_ = prev; }

  void ageIs(time_t age)  { last_refreshed_ = time(NULL) - age; }
  void latestSeqnoIs(uint16_t seqno) { latest_seqno_ = seqno; }
  void distanceIs(uint16_t dist) { distance_ = dist; }
  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Iterators. */

  iterator neighborsBegin() { return neighbor_nodes_.begin(); }
  iterator neighborsEnd() { return neighbor_nodes_.end(); }
  const_iterator neighborsBegin() const { return neighbor_nodes_.begin(); }
  const_iterator neighborsEnd() const { return neighbor_nodes_.end(); }

 private:
  OSPFNode(const RouterID& router_id);

  /* Data members. */
  RouterID router_id_;
  time_t last_refreshed_;
  uint16_t latest_seqno_;
  uint16_t distance_;

  /* Previous node in the shortest path from the root node to this node. */
  OSPFNode::Ptr prev_;

  /* Map of all neighbors directly attached to this node. */
  Fwk::Map<RouterID,OSPFNeighbor> neighbors_;

  /* Mirror map with direct pointers to neighboring OSPFNodes (rather than
     OSPFNeighbor objects). Used to provide iterators. If space constraints
     become a problem, this can be optimized away by defining custom
     iterators. */
  Fwk::Map<RouterID,OSPFNode> neighbor_nodes_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Operations disallowed. */
  OSPFNode(const OSPFNode&);
  void operator=(const OSPFNode&);
};

#endif
