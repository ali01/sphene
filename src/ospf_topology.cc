#include "ospf_topology.h"

#include <queue>

OSPFTopology::OSPFTopology(OSPFNode::Ptr root_node) : root_node_(root_node) {
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

void
OSPFTopology::nodeIs(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  nodes_[node->routerID()] = node;
}

void
OSPFTopology::nodeDel(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  /* Removing the node from the neighbor lists of all neighbors. */
  OSPFNode::Ptr neighbor;
  OSPFNode::iterator it;
  for (it = node->neighborsBegin(); it != node->neighborsEnd(); ++it) {
    neighbor = it->second;
    neighbor->neighborDel(node);
  }

  nodes_.elemDel(node->routerID());
}

void
OSPFTopology::nodeDel(const RouterID& router_id) {
  OSPFNode::Ptr node = this->node(router_id);
  this->nodeDel(node);
}

void
OSPFTopology::onUpdate() {
  compute_optimal_spanning_tree();
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

    OSPFNode::iterator it = cur_nd->neighborsBegin();
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
