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
OSPFNode::linkIs(OSPFLink::Ptr link, bool bi_directional) {
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

  if (bi_directional) {
    /* Relationship is bi-directional. */
    OSPFLink::Ptr reverse_link = OSPFLink::New(this, link->subnet(),
                                               link->subnetMask());
    node->linkIs(reverse_link, false);
  }
}

void
OSPFNode::linkDel(const RouterID& id, bool bi_directional) {
  OSPFNode::Ptr node = neighbor(id);
  if (node) {
    /* Deleting from both maps. */
    links_.elemDel(id);
    neighbors_.elemDel(id);

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onLinkDel(id);

    if (bi_directional) {
      /* Deletion is bi-directional. */
      node->linkDel(this->routerID(), false);
    }
  }
}
