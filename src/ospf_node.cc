#include "ospf_node.h"

#include "ospf_constants.h"

const OSPFNode::Ptr OSPFNode::kPassiveEndpoint =
  OSPFNode::New(OSPF::kPassiveEndpointID);

OSPFNode::OSPFNode(const RouterID& router_id)
    : router_id_(router_id),
      latest_seqno_(0),
      distance_(0) {}

OSPFLink::Ptr
OSPFNode::activeLink(const RouterID& id) {
  return active_links_.elem(id);
}

OSPFLink::PtrConst
OSPFNode::activeLink(const RouterID& id) const {
  return active_links_.elem(id);
}

OSPFLink::Ptr
OSPFNode::passiveLink(const IPv4Addr& subnet, const IPv4Addr mask) {
  return passive_links_.elem(std::make_pair(subnet, mask));
}

OSPFLink::PtrConst
OSPFNode::passiveLink(const IPv4Addr& subnet, const IPv4Addr mask) const {
  return passive_links_.elem(std::make_pair(subnet, mask));
}

bool
OSPFNode::isPassiveEndpoint() const {
  return routerID() == OSPF::kPassiveEndpointID;
}

void
OSPFNode::linkIs(OSPFLink::Ptr link, bool commit) {
  if (link == NULL)
    return;

  oneWayLinkIs(link, commit);

  /* Relationship is bi-directional. */
  OSPFLink::Ptr reverse_link;
  if (isPassiveEndpoint())
    reverse_link = OSPFLink::NewPassive(link->subnet(), link->subnetMask());
  else
    reverse_link = OSPFLink::New(this, link->subnet(), link->subnetMask());

  link->node()->oneWayLinkIs(reverse_link, commit);
}

void
OSPFNode::activeLinkDel(const RouterID& id, bool commit) {
  if (id == routerID())
    return;

  /* Deletion is bi-directional. */
  OSPFLink::Ptr link = activeLink(id);
  if (link) {
    OSPFNode::Ptr node = link->node();
    node->oneWayActiveLinkDel(this->routerID(), commit);
  }

  oneWayActiveLinkDel(id, commit);
}

void
OSPFNode::passiveLinkDel(const IPv4Addr& subnet, const IPv4Addr& mask,
                         bool commit) {
  /* Deletion is bi-directional. */
  OSPFLink::Ptr link = passiveLink(subnet, mask);
  if (link) {
    OSPFNode::Ptr node = link->node();
    node->oneWayPassiveLinkDel(subnet, mask, commit);
  }

  oneWayPassiveLinkDel(subnet, mask, commit);
}

/* OSPFNode private member functions. */

void
OSPFNode::oneWayLinkIs(OSPFLink::Ptr link, bool commit) {
  if (link == NULL)
    return;

  if (!link->nodeIsPassiveEndpoint() && link->nodeRouterID() == routerID()) {
    /* Do not add links to oneself. */
    return;
  }

  OSPFLink::Ptr prev_link;
  if (link->nodeIsPassiveEndpoint()) {
    IPv4Subnet subnet = std::make_pair(link->subnet(), link->subnetMask());
    prev_link = passiveLink(link->subnet(), link->subnetMask());
    passive_links_[subnet] = link;

  } else {
    RouterID nd_id = link->nodeRouterID();
    prev_link = activeLink(nd_id);
    active_links_[nd_id] = link;
  }

  /* Refresh time since last LSU. */
  link->timeSinceLSUIs(0);

  /* Signal notifiee. */
  if (notifiee_ && (prev_link == NULL || *link != *prev_link))
    notifiee_->onLink(this, link, commit);
}

void
OSPFNode::oneWayActiveLinkDel(const RouterID& id, bool commit) {
  OSPFLink::Ptr link = activeLink(id);
  if (link) {
    active_links_.elemDel(id);

    /* Signal notifiee. */
    if (notifiee_)
      notifiee_->onLinkDel(this, link, commit);
  }
}

void
OSPFNode::oneWayPassiveLinkDel(const IPv4Addr& subnet, const IPv4Addr& mask,
                               bool commit) {
  OSPFLink::Ptr link = passiveLink(subnet, mask);
  if (link) {
    IPv4Subnet subnet_key = std::make_pair(subnet, mask);
    passive_links_.elemDel(subnet_key);

    /* Signaling notifiee. */
    if (notifiee_)
      notifiee_->onLinkDel(this, link, commit);
  }
}
