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

  /* Notification support. */
  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported. */

    /* Signaled whenever the dirty attribute
       transitions from TRUE to FALSE. */
    virtual void onDirtyCleared() {}

   protected:
    Notifiee() {}
    virtual ~Notifiee() {}

   private:
    /* Operations disallowed. */
    Notifiee(const Notifiee&);
    void operator=(const Notifiee&);
  };

  /* Accessors. */

  OSPFNode::Ptr node(const RouterID& router_id);
  OSPFNode::PtrConst node(const RouterID& router_id) const;

  OSPFNode::Ptr nextHop(const RouterID& router_id);
  OSPFNode::PtrConst nextHop(const RouterID& router_id) const;

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  size_t nodes() const { return nodes_.size(); }
  bool dirty() const { return dirty_; }

  /* Mutators. */

  void nodeIs(OSPFNode::Ptr node, bool commit=true);
  void nodeDel(OSPFNode::Ptr node, bool commit=true);
  void nodeDel(const RouterID& router_id, bool commit=true);

  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Iterators. */

  iterator nodesBegin() { return nodes_.begin(); }
  iterator nodesEnd() { return nodes_.end(); }
  const_iterator nodesBegin() const { return nodes_.begin(); }
  const_iterator nodesEnd() const { return nodes_.end(); }

  /* Signals. */
  /* onPossibleUpdate signal: Recomputes shortest-path spanning tree if dirty.*/
  void onPossibleUpdate();

 private:
  OSPFTopology(OSPFNode::Ptr root_node);

  /* Reactor to OSPFNode notifications. */
  class NodeReactor : public OSPFNode::Notifiee {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    static Ptr New(OSPFTopology* _t) {
      return new NodeReactor(_t);
    }

    void onLink(OSPFNode::Ptr node, OSPFLink::Ptr link, bool commit);
    void onLinkDel(OSPFNode::Ptr node, const RouterID& rid, bool commit);

   private:
    NodeReactor(OSPFTopology* _t) : topology_(_t) {}

    /* Data members. */
    OSPFTopology* topology_; /* Weak ptr to prevent circular reference. */

    /* Operations disallowed. */
    NodeReactor(const NodeReactor&);
    void operator=(const NodeReactor&);
  };

  /* Private member functions. */

  void compute_optimal_spanning_tree();
  void dirtyIs(bool status);

  /* Depending on COMMIT, recomputes the
     spanning tree or just sets the dirty bit. */
  void process_update(bool commit);

  static OSPFNode::Ptr min_dist_node(const Fwk::Map<RouterID,OSPFNode>& map);

  /* Data members. */

  Fwk::Map<RouterID,OSPFNode> nodes_;
  OSPFNode::Ptr root_node_;
  NodeReactor::Ptr node_reactor_;

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* Dirty bit. */
  bool dirty_;

  /* Operations disallowed. */
  OSPFTopology(const OSPFTopology&);
  void operator=(const OSPFTopology&);
};

#endif
