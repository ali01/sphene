#include "ospf_node.h"

#include "ospf_constants.h"

const OSPFNode::Ptr OSPFNode::kPassiveEndpoint =
  OSPFNode::New(OSPF::kPassiveEndpointID);

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

bool
OSPFNode::isPassiveEndpoint() const {
  return routerID() == OSPF::kPassiveEndpointID;
}

void
OSPFNode::linkIs(OSPFLink::Ptr link, bool commit) {
  if (link->nodeRouterID() == routerID()) {
    /* Should not establish links to self. */
    return;
  }

  oneWayLinkIs(link, commit);

  /* Relationship is bi-directional. */
  if (link) {
    OSPFLink::Ptr reverse_link = OSPFLink::New(this, link->subnet(),
                                               link->subnetMask());
    link->node()->oneWayLinkIs(reverse_link, commit);
  }
}

void
OSPFNode::linkDel(const RouterID& id, bool commit) {
  if (id == routerID())
    return;

  /* Deletion is bi-directional. */
  OSPFLink::Ptr link = this->link(id);
  OSPFNode::Ptr node = link->node();
  if (node)
    node->oneWayLinkDel(this->routerID(), commit);

  oneWayLinkDel(id, commit);
}

/* OSPFNode private member functions. */

void
OSPFNode::oneWayLinkIs(OSPFLink::Ptr link, bool commit) {
  if (link == NULL)
    return;

  OSPFNode::Ptr node = link->node();
  RouterID nd_id = node->routerID();
  OSPFLink::Ptr prev_link = this->link(nd_id);

  /* Adding to OSPFLink pointer map. */
  links_[nd_id] = link;

  /* Refresh time since last LSU. */
  link->timeSinceLSUIs(0);

  /* Signal notifiee. */
  if (notifiee_ && (prev_link == NULL || *link != *prev_link))
    notifiee_->onLink(this, link, commit);
}

void
OSPFNode::oneWayLinkDel(const RouterID& id, bool commit) {
  OSPFLink::Ptr link = this->link(id);
  OSPFNode::Ptr node = link->node();
  if (node) {
    /* Deleting from both maps. */
    links_.elemDel(id);

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onLinkDel(this, id, commit);
  }
}
