#ifndef OSPF_NODE_H_VKYMXJVI
#define OSPF_NODE_H_VKYMXJVI

#include <ctime>

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"

class OSPFNode : public Fwk::PtrInterface<OSPFNode> {
 public:
  typedef Fwk::Ptr<const OSPFNode> PtrConst;
  typedef Fwk::Ptr<OSPFNode> Ptr;

  typedef Fwk::Map<uint32_t,OSPFNode>::iterator iterator;
  typedef Fwk::Map<uint32_t,OSPFNode>::const_iterator const_iterator;

  static const uint16_t kMaxDistance = 0xffff;

  static Ptr New(uint32_t router_id) {
    return new OSPFNode(router_id, IPv4Addr::kZero);
  }

  static Ptr New(uint32_t router_id, IPv4Addr subnet) {
    return new OSPFNode(router_id, subnet);
  }

  /* Accessors. */ 

  uint32_t routerID() const { return router_id_; }
  IPv4Addr subnet() const { return subnet_; }
  IPv4Addr subnetMask() const { return subnet_mask_; }

  OSPFNode::Ptr neighbor(uint32_t id);
  OSPFNode::PtrConst neighbor(uint32_t id) const;

  time_t age() const { return time(NULL) - last_refreshed_; }
  uint16_t latestSeqno() const { return latest_seqno_; }
  uint16_t distance() const { return distance_; }

  /* Mutators. */

  void subnetIs(IPv4Addr subnet) { subnet_ = subnet; }
  void subnetMaskIs(IPv4Addr mask) { subnet_mask_ = mask; }

  void neighborIs(OSPFNode::Ptr node);
  void neighborDel(uint32_t id);
  void neighborDel(OSPFNode::Ptr node);

  void ageIs(time_t age)  { last_refreshed_ = time(NULL) - age; }
  void latestSeqnoIs(uint16_t seqno) { latest_seqno_ = seqno; }
  void distanceIs(uint16_t dist) { distance_ = dist; }

  /* Iterators. */

  iterator neighborsBegin() { return neighbors_.begin(); }
  iterator neighborsEnd() { return neighbors_.end(); }
  const_iterator neighborsBegin() const { return neighbors_.begin(); }
  const_iterator neighborsEnd() const { return neighbors_.end(); }

 private:
  OSPFNode(uint32_t router_id, IPv4Addr subnet);

  /* Data members. */
  uint32_t router_id_;
  IPv4Addr subnet_;
  IPv4Addr subnet_mask_;
  time_t last_refreshed_;
  uint16_t latest_seqno_;
  uint16_t distance_;

  /* Map of all neighbors directly attached to this node. */
  Fwk::Map<uint32_t,OSPFNode> neighbors_;

  /* Operations disallowed. */
  OSPFNode(const OSPFNode&);
  void operator=(const OSPFNode&);
};

#endif
