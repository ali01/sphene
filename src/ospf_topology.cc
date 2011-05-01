#include "ospf_topology.h"

OSPFTopology::OSPFTopology(OSPFNode::Ptr root_node) : root_node_(root_node) {
  this->nodeIs(root_node_);
}

OSPFNode::Ptr
OSPFTopology::node(uint32_t router_id) {
  return nodes_.elem(router_id);
}

OSPFNode::PtrConst
OSPFTopology::node(uint32_t router_id) const {
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
OSPFTopology::nodeDel(uint32_t router_id) {
  OSPFNode::Ptr node = this->node(router_id);
  this->nodeDel(node);
}
