#include "ospf_link.h"

#include "ospf_node.h"

OSPFLink::Ptr
OSPFLink::New(OSPFNode::Ptr neighbor,
              const IPv4Addr& subnet,
              const IPv4Addr& subnet_mask) {
  return new OSPFLink(neighbor, subnet, subnet_mask);
}

OSPFLink::OSPFLink(OSPFNode::Ptr neighbor,
                   const IPv4Addr& subnet,
                   const IPv4Addr& subnet_mask)
    : node_(neighbor),
      subnet_(subnet & subnet_mask),
      subnet_mask_(subnet_mask),
      last_lsu_(time(NULL)) {}

OSPFNode::PtrConst
OSPFLink::node() const {
  return node_;
}

OSPFNode::Ptr
OSPFLink::node() {
  return node_;
}

const RouterID&
OSPFLink::nodeRouterID() const {
  return node_->routerID();
}

bool
OSPFLink::nodeIsEndpoint() const {
  return node_->isEndpoint();
}

bool
OSPFLink::operator==(const OSPFLink& other) const {
  if (this->node() != other.node()) /* Node pointer equivalence */
    return false;

  if (this->subnet() != other.subnet())
    return false;

  if (this->subnetMask() != other.subnetMask())
    return false;

  return true;
}

bool
OSPFLink::operator!=(const OSPFLink& other) const {
  return !(other == *this);
}
