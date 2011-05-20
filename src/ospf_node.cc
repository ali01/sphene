#include "ospf_node.h"

#include <ostream>
#include <sstream>
using std::stringstream;

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

string
OSPFNode::str() const {
  RouterID upstream_id = upstreamNode() ? upstreamNode()->routerID() : 0;

  stringstream ss;
  ss << "  Router ID:   " << routerID() << "\n";
  ss << "  Upstream ID: " << upstream_id << "\n";
  ss << "  Distance:    " << distance() << "\n";
  ss << "  Links:       " << links() << "\n";
  ss << "    Active Neighbors:\n";
  for (const_lna_iter it = activeLinksBegin(); it != activeLinksEnd(); ++it) {
    OSPFLink::PtrConst link = it->second;
    ss << *link;
  }

  ss << "    Passive Neighbors:\n";
  for (const_lnp_iter it = passiveLinksBegin(); it != passiveLinksEnd(); ++it) {
    OSPFLink::PtrConst link = it->second;
    ss << *link;
  }

  ss << "\n";

  return ss.str();
}

void
OSPFNode::linkIs(OSPFLink::Ptr link) {
  if (link == NULL)
    return;

  /* Relationship is bi-directional. */
  OSPFLink::Ptr reverse_link =
    OSPFLink::New(this, link->subnet(), link->subnetMask());

  bool notify_cond_one = link->node()->oneWayLinkIs(reverse_link);
  bool notify_cond_two = oneWayLinkIs(link);

  /* Signal notifiee. */
  if (notifiee_ && (notify_cond_one || notify_cond_two))
    notifiee_->onLink(this, link);
}

void
OSPFNode::activeLinkDel(const RouterID& id) {
  if (id == routerID())
    return;

  bool notify_cond_one = false;
  bool notify_cond_two = false;

  /* Deletion is bi-directional. */
  OSPFLink::Ptr link = activeLink(id);
  if (link) {
    OSPFNode::Ptr node = link->node();
    notify_cond_one = node->oneWayActiveLinkDel(this->routerID());
  }

  notify_cond_two = oneWayActiveLinkDel(id);

  if (notifiee_ && (notify_cond_one || notify_cond_two))
    notifiee_->onLinkDel(this, link);
}

void
OSPFNode::passiveLinkDel(const IPv4Addr& subnet, const IPv4Addr& mask) {
  bool notify_cond_one = false;
  bool notify_cond_two = false;

  /* Deletion is bi-directional. */
  OSPFLink::Ptr link = passiveLink(subnet, mask);
  if (link) {
    OSPFNode::Ptr node = link->node();
    notify_cond_one = node->oneWayPassiveLinkDel(subnet, mask);
  }

  notify_cond_two = oneWayPassiveLinkDel(subnet, mask);

  if (notifiee_ && (notify_cond_one || notify_cond_two))
    notifiee_->onLinkDel(this, link);
}

/* OSPFNode private member functions. */

bool
OSPFNode::oneWayLinkIs(OSPFLink::Ptr link) {
  if (link == NULL)
    return false;

  if (this->isPassiveEndpoint()) {
    /* Don't add links to passive endpoints. */
    return false;
  }

  if (!link->nodeIsPassiveEndpoint() && link->nodeRouterID() == routerID()) {
    /* Do not add links to oneself. */
    return false;
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

  return (prev_link == NULL || *link != *prev_link);
}

bool
OSPFNode::oneWayActiveLinkDel(const RouterID& id) {
  OSPFLink::Ptr link = activeLink(id);
  if (link) {
    active_links_.elemDel(id);
    return true;
  }

  return false;
}

bool
OSPFNode::oneWayPassiveLinkDel(const IPv4Addr& subnet, const IPv4Addr& mask) {
  OSPFLink::Ptr link = passiveLink(subnet, mask);
  if (link) {
    IPv4Subnet subnet_key = std::make_pair(subnet, mask);
    passive_links_.elemDel(subnet_key);
    return true;
  }

  return false;
}

ostream&
operator<<(ostream& out, const OSPFNode& node) {
  return out << node.str();
}

Fwk::Log::LogStream::Ptr
operator<<(Fwk::Log::LogStream::Ptr ls, const OSPFNode& node) {
  return ls << node.str();
}
