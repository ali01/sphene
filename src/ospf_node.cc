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
OSPFNode::linkIs(OSPFLink::Ptr link) {
  oneWayLinkIs(link);

  /* Relationship is bi-directional. */
  if (link) {
    OSPFLink::Ptr reverse_link = OSPFLink::New(this, link->subnet(),
                                               link->subnetMask());
    link->node()->oneWayLinkIs(reverse_link);
  }
}

void
OSPFNode::linkDel(const RouterID& id) {
  oneWayLinkDel(id);

  /* Deletion is bi-directional. */
  OSPFNode::Ptr node = neighbor(id);
  if (node)
    node->oneWayLinkDel(this->routerID());
}

/* OSPFNode private member functions. */

void
OSPFNode::oneWayLinkIs(OSPFLink::Ptr link) {
  if (link == NULL)
    return;

  OSPFNode::Ptr node = link->node();
  RouterID nd_id = node->routerID();

  OSPFLink::Ptr prev_link = links_.elem(nd_id);

  /* Adding to OSPFLink pointer map. */
  links_[nd_id] = link;

  /* Adding to direct OSPFNode pointer map. */
  neighbors_[nd_id] = node;

  /* Refresh time since last LSU. */
  link->timeSinceLSUIs(0);

  /* Signal notifiee. */
  if (notifiee_ && (prev_link == NULL || *link != *prev_link))
    notifiee_->onLink(nd_id);
}

void
OSPFNode::oneWayLinkDel(const RouterID& id) {
  OSPFNode::Ptr node = neighbor(id);
  if (node) {
    /* Deleting from both maps. */
    links_.elemDel(id);
    neighbors_.elemDel(id);

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onLinkDel(id);
  }
}
