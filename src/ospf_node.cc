#include "ospf_node.h"

OSPFNode::OSPFNode(const RouterID& router_id)
    : router_id_(router_id),
      latest_seqno_(0),
      distance_(0) {}

OSPFLink::Ptr
OSPFNode::link(const RouterID& id) {
  return links_.elem(id);
}

OSPFLink::PtrConst
OSPFNode::link(const RouterID& id) const {
  return links_.elem(id);
}

OSPFNode::Ptr
OSPFNode::neighbor(const RouterID& id) {
  return neighbors_.elem(id);
}

OSPFNode::PtrConst
OSPFNode::neighbor(const RouterID& id) const {
  OSPFNode* self = const_cast<OSPFNode*>(this);
  return self->neighbor(id);
}

void
OSPFNode::linkIs(OSPFNode::Ptr node,
                 const IPv4Addr& subnet,
                 const IPv4Addr& subnet_mask) {
  if (node == NULL)
    return;

  RouterID nd_id = node->routerID();
  OSPFLink::Ptr nbr_prev = links_.elem(nd_id);
  if (nbr_prev == NULL
      || nbr_prev->node() != node
      || nbr_prev->subnet() != subnet
      || nbr_prev->subnetMask() != subnet_mask) {

    /* Adding to OSPFLink pointer map. */
    OSPFLink::Ptr nbr_new = OSPFLink::New(node, subnet, subnet_mask);
    links_[nd_id] = nbr_new;

    /* Adding to direct OSPFNode pointer map. */
    neighbors_[nd_id] = node;

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onLink(nd_id);
  } else {
    nbr_prev->timeSinceLSUIs(0);
  }

  /* Relationship is bi-directional. */
  node->linkIs(this, subnet, subnet_mask);
}

void
OSPFNode::linkDel(const RouterID& id) {
  OSPFNode::Ptr node = neighbor(id);
  if (node) {
    /* Deletion is bi-directional. */
    node->linkDel(this->routerID());

    /* Deleting from both maps. */
    links_.elemDel(id);
    neighbors_.elemDel(id);

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onLinkDel(id);
  }
}
