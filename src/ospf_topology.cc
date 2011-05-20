#include "ospf_topology.h"

#include <sstream>
using std::stringstream;

#include "fwk/deque.h"
#include "fwk/log.h"

#include "ospf_constants.h"

/* Static global log instance */
static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("OSPFTopology");


OSPFTopology::OSPFTopology(OSPFNode::Ptr root_node)
    : root_node_(root_node),
      node_reactor_(NodeReactor::New(this)),
      dirty_(false) {
  root_node_->notifieeIs(node_reactor_);
}

OSPFNode::PtrConst
OSPFTopology::rootNode() const {
  return root_node_;
}

OSPFNode::Ptr
OSPFTopology::rootNode() {
  return root_node_;
}

OSPFNode::Ptr
OSPFTopology::node(const RouterID& router_id) {
  if (router_id == root_node_->routerID())
    return root_node_;

  return nodes_.elem(router_id);
}

OSPFNode::PtrConst
OSPFTopology::node(const RouterID& router_id) const {
  OSPFTopology* self = const_cast<OSPFTopology*>(this);
  return self->node(router_id);
}

OSPFNode::Ptr
OSPFTopology::nextHop(const RouterID& router_id) {
  OSPFNode::Ptr next_hop = NULL;

  OSPFNode::Ptr nd = node(router_id);
  for (;;) {
    if (nd == NULL)
      break;

    if (nd->upstreamNode() == root_node_) {
      next_hop = nd;
      break;
    }

    nd = nd->upstreamNode();
  }

  return next_hop;
}

OSPFNode::PtrConst
OSPFTopology::nextHop(const RouterID& router_id) const {
  OSPFTopology* self = const_cast<OSPFTopology*>(this);
  return self->nextHop(router_id);
}

void
OSPFTopology::nodeIs(OSPFNode::Ptr node, bool commit) {
  if (node == NULL)
    return;

  RouterID nd_id = node->routerID();
  if (nd_id == OSPF::kInvalidRouterID) {
    ELOG << "Attempt to insert a node with invalid ID into topology.";
    return;
  }

  if (node->isPassiveEndpoint()) {
    /* Do not insert passive endpoints into the topology. */
    return;
  }

  if (nd_id == root_node_->routerID()) {
    ELOG << "Attempt to insert the root node into its own topology.";
    return;
  }

  OSPFNode::Ptr node_prev = nodes_.elem(nd_id);
  if (node_prev != node) {
    nodes_[nd_id] = node;

    /* Subscribing from notifications. */
    node->notifieeIs(node_reactor_);

    /* Depending on COMMIT, recompute spanning tree or just set dirty bit. */
    process_update(commit);
  }
}

void
OSPFTopology::nodeDel(OSPFNode::Ptr node, bool commit) {
  if (node == NULL)
    return;

  nodeDel(node->routerID(), commit);
}

void
OSPFTopology::nodeDel(const RouterID& router_id, bool commit) {
  OSPFNode::Ptr node = nodes_.elem(router_id);
  if (node) {
    nodes_.elemDel(router_id);

    /* Unsubscribing from notifications. */
    node->notifieeIs(NULL);

    /* Removing the node from the neighbor lists of all neighbors. Neighbors
       vector prevents indirect deletion during iteration. */
    Fwk::Deque<OSPFNode::Ptr> neighbors;
    for (OSPFNode::lna_iter it = node->activeLinksBegin();
         it != node->activeLinksEnd(); ++it) {
      OSPFLink::Ptr link = it->second;
      neighbors.pushBack(link->node());
    }

    for (OSPFNode::lnp_iter it = node->passiveLinksBegin();
         it != node->passiveLinksEnd(); ++it) {
      OSPFLink::Ptr link = it->second;
      neighbors.pushBack(link->node());
    }

    for (Fwk::Deque<OSPFNode::Ptr>::const_iterator it = neighbors.begin();
         it != neighbors.end(); ++it) {
      OSPFNode::Ptr neighbor = *it;
      neighbor->activeLinkDel(router_id, false);
    }

    /* Depending on COMMIT, recompute spanning tree or just set dirty bit. */
    process_update(commit);
  }
}

void
OSPFTopology::onPossibleUpdate() {
  if (dirty())
    compute_optimal_spanning_tree();
}

string
OSPFTopology::str() const {
  stringstream ss;
  ss << "OSPF Topology\n";
  ss << "  Root Node:\n";
  ss << *root_node_;

  const_iterator it;
  for (it = nodesBegin(); it != nodesEnd(); ++it) {
    OSPFNode::Ptr node = it->second;
    ss << *node;
  }

  return ss.str();
}

/* Implementation of Dijkstra's algorithm. */
void
OSPFTopology::compute_optimal_spanning_tree() {
  ILOG << "Computing optimal spanning tree";

  /* Set of all nodes: shallow copy of nodes_. */
  Fwk::Map<RouterID,OSPFNode> node_set(nodes_);
  node_set[root_node_->routerID()] = root_node_;

  /* Initializing distance to all nodes to kMaxDistance. */
  for (iterator it = node_set.begin(); it != node_set.end(); ++it) {
    OSPFNode::Ptr node = it->second;
    node->distanceIs(OSPFNode::kMaxDistance);
    node->upstreamNodeIs(NULL);
  }

  /* Initializing distance to self to 0 */
  root_node_->distanceIs(0);

  while (node_set.size() > 0) {
    OSPFNode::Ptr cur_nd = min_dist_node(node_set);
    if (cur_nd->distance() == OSPFNode::kMaxDistance) {
      /* All remaining nodes are inaccessible from root_node_. */
      break;
    }

    node_set.elemDel(cur_nd->routerID());

    for (OSPFNode::lna_iter it = cur_nd->activeLinksBegin();
         it != cur_nd->activeLinksEnd(); ++it) {
      OSPFLink::Ptr link = it->second;
      OSPFNode::Ptr neighbor = link->node();
      neighbor = node_set.elem(neighbor->routerID());
      if (neighbor) {
        /* Neighboring node, NEIGHBOR, has not been visited yet.
         * Namely, it still exists in node_set. We can continue. */
        uint16_t alt_dist = cur_nd->distance() + 1;
        if (alt_dist < neighbor->distance()) {
          neighbor->distanceIs(alt_dist);
          neighbor->upstreamNodeIs(cur_nd);
        }
      }
    }
  }

  /* Resetting topology dirty bit. */
  dirtyIs(false);
}

// TODO(ali): make use of a heap instead.
OSPFNode::Ptr
OSPFTopology::min_dist_node(const Fwk::Map<RouterID,OSPFNode>& map) {
  OSPFNode::Ptr min_nd = NULL;
  for (const_iterator it = map.begin(); it != map.end(); ++it) {
    OSPFNode::Ptr cur_nd = it->second;
    if (min_nd == NULL || cur_nd->distance() < min_nd->distance())
      min_nd = cur_nd;
  }

  return min_nd;
}

void
OSPFTopology::dirtyIs(bool status) {
  /* Notify if dirty_ transitions from true to false. */
  bool notify = (dirty_ && !status) ? true : false;
  dirty_ = status;

  /* Signal notifiee. */
  if (notifiee_ && notify)
    notifiee_->onDirtyCleared();
}

void
OSPFTopology::process_update(bool commit) {
  dirtyIs(true);

  if (commit)
    compute_optimal_spanning_tree();
}

/* OSPFTopology::NodeReactor */

void
OSPFTopology::NodeReactor::onLink(OSPFNode::Ptr node, OSPFLink::Ptr link,
                                  bool commit) {
  topology_->nodeIs(link->node(), false);
  topology_->process_update(commit);
}

void
OSPFTopology::NodeReactor::onLinkDel(OSPFNode::Ptr node, OSPFLink::Ptr link,
                                     bool commit) {
  topology_->process_update(commit);
}
