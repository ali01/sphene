#include "ospf_topology.h"

#include <queue>

OSPFTopology::OSPFTopology(OSPFNode::Ptr root_node)
    : root_node_(root_node), node_reactor_(NodeReactor::New(this)) {
  this->nodeIs(root_node_);
}

OSPFNode::Ptr
OSPFTopology::node(const RouterID& router_id) {
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

void
OSPFTopology::nodeIs(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  RouterID nd_id = node->routerID();
  OSPFNode::Ptr node_prev = nodes_.elem(nd_id);
  if (node_prev != node) {
    nodes_[nd_id] = node;

    /* Subscribing from notifications. */
    node->notifieeIs(node_reactor_);

    /* Setting topology dirty bit. */
    dirtyIs(true);
  }
}

void
OSPFTopology::nodeDel(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  nodeDel(node->routerID());
}

void
OSPFTopology::nodeDel(const RouterID& router_id) {
  OSPFNode::Ptr node = nodes_.elem(router_id);
  if (node) {
    nodes_.elemDel(router_id);

    /* Unsubscribing from notifications. */
    node->notifieeIs(NULL);

    /* Setting topology dirty bit. */
    dirtyIs(true);
  }

  /* Removing the node from the neighbor lists of all neighbors. */
  OSPFNode::nb_iter it;
  for (it = node->neighborsBegin(); it != node->neighborsEnd(); ++it) {
    OSPFNode::Ptr neighbor = it->second;
    neighbor->linkDel(node);
  }
}

void
OSPFTopology::onUpdate() {
  if (dirty()) {
    compute_optimal_spanning_tree();

    /* Resetting topology dirty bit. */
    dirtyIs(false);

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onDirtyCleared();
  }
}

/* Implementation of Dijkstra's algorithm. */
void
OSPFTopology::compute_optimal_spanning_tree() {
  /* Set of all nodes: shallow copy of nodes_. */
  Fwk::Map<RouterID,OSPFNode> node_set(nodes_);

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
