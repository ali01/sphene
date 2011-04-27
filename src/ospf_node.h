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

  static Ptr New() {
    return new OSPFNode();
  }

  /* Accessors. */ 

  uint32_t routerID() const { return router_id_; }
  IPv4Addr subnet() const { return subnet_; }
  IPv4Addr subnetMask() const { return subnet_mask_; }

  OSPFNode::Ptr neighbor(uint32_t id);
  OSPFNode::PtrConst neighbor(uint32_t id) const;

  OSPFNode::Ptr nextHop() { return next_hop_; }
  OSPFNode::PtrConst nextHop() const { return next_hop_; }

  time_t age() const { return time(NULL) - t_; }

  /* Mutators. */

  void neighborIs(OSPFNode::Ptr node);
  void neighborDel(uint32_t id);
  void neighborDel(OSPFNode::Ptr node);

  void nextHopIs(OSPFNode::Ptr node) { next_hop_ = node; }

  void ageIs(time_t age)  { t_ = time(NULL) - age; }

  /* Iterators. */

  iterator neighborsBegin() { return neighbors_.begin(); }
  iterator neighborsEnd() { return neighbors_.end(); }
  const_iterator neighborsBegin() const { return neighbors_.begin(); }
  const_iterator neighborsEnd() const { return neighbors_.end(); }

 private:
  OSPFNode() {}

  /* Data members. */
  uint32_t router_id_;
  IPv4Addr subnet_;
  IPv4Addr subnet_mask_;
  time_t t_;

  /* Next hop node in shortest path from this router. */
  OSPFNode::Ptr next_hop_;

  /* Map of all neighbors directly attached to this node. */
  Fwk::Map<uint32_t,OSPFNode> neighbors_;

  /* Operations disallowed. */
  OSPFNode(const OSPFNode&);
  void operator=(const OSPFNode&);
};


#endif
