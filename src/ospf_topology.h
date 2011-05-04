#ifndef OSPF_TOPOLOGY_H_I3MK5NYZ
#define OSPF_TOPOLOGY_H_I3MK5NYZ

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ospf_node.h"
#include "ospf_types.h"


class OSPFTopology : public Fwk::PtrInterface<OSPFTopology> {
 public:
  typedef Fwk::Ptr<const OSPFTopology> PtrConst;
  typedef Fwk::Ptr<OSPFTopology> Ptr;

  typedef Fwk::Map<RouterID,OSPFNode>::iterator iterator;
  typedef Fwk::Map<RouterID,OSPFNode>::const_iterator const_iterator;

  static Ptr New(OSPFNode::Ptr root_node) {
    return new OSPFTopology(root_node);
  }

  class NodeReactor : public OSPFNode::Notifiee {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    static Ptr New(OSPFTopology::Ptr _t) {
      return new NodeReactor(_t);
    }

    void onNeighbor(const RouterID& id) {}
    void onNeighborDel(const RouterID& id) {}

   private:
    NodeReactor(OSPFTopology::Ptr _t) : topology_(_t.ptr()) {}

    /* Data members. */
    OSPFTopology* topology_; /* Weak ptr to prevent circular reference. */

    /* Operations disallowed. */
    NodeReactor(const NodeReactor&);
    void operator=(const NodeReactor&);
  };

  OSPFNode::Ptr node(const RouterID& router_id);
  OSPFNode::PtrConst node(const RouterID& router_id) const;

  void nodeIs(OSPFNode::Ptr node);
  void nodeDel(OSPFNode::Ptr node);
  void nodeDel(const RouterID& router_id);

  iterator nodesBegin() { return nodes_.begin(); }
  iterator nodesEnd() { return nodes_.end(); }
  const_iterator nodesBegin() const { return nodes_.begin(); }
  const_iterator nodesEnd() const { return nodes_.end(); }

  /* Recomputes shortest-path spanning tree. */
  void onUpdate();

 private:
  OSPFTopology(OSPFNode::Ptr root_node);

  void compute_optimal_spanning_tree();
  static OSPFNode::Ptr min_dist_node(const Fwk::Map<RouterID,OSPFNode>& map);

  /* Data members. */
  Fwk::Map<RouterID,OSPFNode> nodes_;
  OSPFNode::Ptr root_node_;
  NodeReactor::Ptr node_reactor_;

  /* Operations disallowed. */
  OSPFTopology(const OSPFTopology&);
  void operator=(const OSPFTopology&);
};

#endif
