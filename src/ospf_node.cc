#include "ospf_node.h"

OSPFNode::OSPFNode(const RouterID& router_id)
    : router_id_(router_id),
      last_refreshed_(time(NULL)),
      latest_seqno_(0),
      distance_(0) {}

OSPFNode::Ptr
OSPFNode::neighbor(const RouterID& id) {
  return neighbor_nodes_.elem(id);
}

OSPFNode::PtrConst
OSPFNode::neighbor(const RouterID& id) const {
  OSPFNode* self = const_cast<OSPFNode*>(this);
  return self->neighbor(id);
}

IPv4Addr
OSPFNode::neighborSubnet(const RouterID& neighbor_id) const {
  OSPFNeighbor::PtrConst nbr = neighbors_.elem(neighbor_id);
  if (nbr)
    return nbr->subnet();

  return IPv4Addr::kZero;
}

IPv4Addr
OSPFNode::neighborSubnetMask(const RouterID& neighbor_id) const {
  OSPFNeighbor::PtrConst nbr = neighbors_.elem(neighbor_id);
  if (nbr)
    return nbr->subnetMask();

  return IPv4Addr::kMax;
}

void
OSPFNode::neighborIs(OSPFNode::Ptr node,
                     const IPv4Addr& subnet,
                     const IPv4Addr& subnet_mask) {
  if (node == NULL)
    return;

  RouterID nd_id = node->routerID();

  /* Adding to direct OSPFNode pointer map. */
  neighbor_nodes_[nd_id] = node;

  /* Adding to OSPFNeighbor pointer map. */
  OSPFNeighbor::Ptr ospf_nbr = OSPFNeighbor::New(node, subnet, subnet_mask);
  neighbors_[nd_id] = ospf_nbr;

  /* Relationship is bi-directional. */
  // TODO(ali): device confirmation mechanism.
  node->neighborIs(this, subnet, subnet_mask);
}

void
OSPFNode::neighborDel(const RouterID& id) {
  /* Deletion is bi-directional. */
  OSPFNode::Ptr node = neighbor(id);
  if (node)
    node->neighborDel(this);

  /* Deleting from both maps. */
  neighbors_.elemDel(id);
  neighbor_nodes_.elemDel(id);
}

void
OSPFNode::neighborDel(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  neighborDel(node->routerID());
}
