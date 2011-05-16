#include "ospf_topology.h"

#include <cstdlib>
#include <ctime>
#include <limits>

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

  /* Seeding random number generator. */
  srand48(time(NULL));
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

    if (nd->prev() == root_node_) {
      next_hop = nd;
      break;
    }

    nd = nd->prev();
  }

  return next_hop;
}

OSPFNode::PtrConst
OSPFTopology::nextHop(const RouterID& router_id) const {
  OSPFTopology* self = const_cast<OSPFTopology*>(this);
  return self->nextHop(router_id);
}

RouterID
OSPFTopology::routerIDNew() const {
  RouterID rid;

  do {
    rid = (RouterID)(lrand48() % std::numeric_limits<RouterID>::max());
  } while(rid == 0
          || rid == OSPF::kInvalidRouterID
          || this->node(rid) != NULL);

  return rid;
}

void
OSPFTopology::nodeIs(OSPFNode::Ptr node, bool commit) {
  if (node == NULL)
    return;

  RouterID nd_id = node->routerID();
  if (nd_id == root_node_->routerID()) {
    ELOG << "Attempt to insert the root node into its own topology.";
    return;
  }

  if (nd_id == OSPF::kInvalidRouterID || nd_id == 0) {
    ELOG << "Attempt to insert a node into topology with invalid ID of "
         << nd_id;
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
    for (OSPFNode::nb_iter it = node->neighborsBegin();
         it != node->neighborsEnd(); ++it) {
      OSPFNode::Ptr neighbor = it->second;
      neighbors.pushBack(neighbor);
    }

    for (Fwk::Deque<OSPFNode::Ptr>::const_iterator it = neighbors.begin();
         it != neighbors.end(); ++it) {
      OSPFNode::Ptr neighbor = *it;
      neighbor->linkDel(router_id);
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

/* Implementation of Dijkstra's algorithm. */
void
OSPFTopology::compute_optimal_spanning_tree() {
  /* Set of all nodes: shallow copy of nodes_. */
  Fwk::Map<RouterID,OSPFNode> node_set(nodes_);
  node_set[root_node_->routerID()] = root_node_;

  /* Initializing distance to all nodes to kMaxDistance. */
  for (iterator it = node_set.begin(); it != node_set.end(); ++it) {
    OSPFNode::Ptr node = it->second;
    node->distanceIs(OSPFNode::kMaxDistance);
    node->prevIs(NULL);
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

    OSPFNode::nb_iter it = cur_nd->neighborsBegin();
    for (; it != cur_nd->neighborsEnd(); ++it) {
      OSPFNode::Ptr neighbor = it->second;
      neighbor = node_set.elem(neighbor->routerID());
      if (neighbor) {
        /* Neighboring node, NEIGHBOR, has not been visited yet.
         * Namely, it still exists in node_set. We can continue. */
        uint16_t alt_dist = cur_nd->distance() + 1;
        if (alt_dist < neighbor->distance()) {
          neighbor->distanceIs(alt_dist);
          neighbor->prevIs(cur_nd);
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
OSPFTopology::NodeReactor::onLink(OSPFNode::Ptr node,
                                  OSPFLink::Ptr link,
                                  bool commit) {
  topology_->process_update(commit);
}

void
OSPFTopology::NodeReactor::onLinkDel(OSPFNode::Ptr node,
                                     const RouterID& rid,
                                     bool commit) {
  topology_->process_update(commit);
}
