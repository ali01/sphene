#include "ospf_link.h"

#include <cstdio>
#include <sstream>
using std::stringstream;

#include "ospf_constants.h"
#include "ospf_node.h"

OSPFLink::Ptr
OSPFLink::New(OSPFNode::Ptr neighbor,
              const IPv4Addr& subnet,
              const IPv4Addr& subnet_mask) {
  return new OSPFLink(neighbor, subnet, subnet_mask);
}

/* Link to a passive, non-OSPF subnet/endpoint. */
OSPFLink::Ptr
OSPFLink::NewPassive(const IPv4Addr& subnet,
                     const IPv4Addr& subnet_mask) {
  return new OSPFLink(subnet, subnet_mask);
}


OSPFLink::OSPFLink(OSPFNode::Ptr neighbor,
                   const IPv4Addr& subnet,
                   const IPv4Addr& subnet_mask)
    : node_(neighbor),
      subnet_(subnet & subnet_mask),
      subnet_mask_(subnet_mask),
      last_lsu_(time(NULL)) {}

OSPFLink::OSPFLink(const IPv4Addr& subnet, const IPv4Addr& mask)
    : node_(OSPFNode::kPassiveEndpoint),
      subnet_(subnet & mask),
      subnet_mask_(mask),
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
OSPFLink::nodeIsPassiveEndpoint() const {
  return node_->isPassiveEndpoint();
}

string
OSPFLink::str() const {
  stringstream ss;
  
  char line_buf[256];
  const char* const format = "      %-16s %-16s %-16s\n";
  const string& subnet_str = subnet();
  const string& mask_str = subnetMask();

  ss << nodeRouterID();

  snprintf(line_buf, sizeof(line_buf), format,
           ss.str().c_str(), subnet_str.c_str(), mask_str.c_str());

  return line_buf;
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

ostream&
operator<<(ostream& out, const OSPFLink& link) {
  return out << link.str();
}
