#include "ospf_gateway.h"

#include "ospf_interface.h"
#include "ospf_node.h"

OSPFGateway::Ptr
OSPFGateway::New(OSPFNode::Ptr neighbor,
                 const IPv4Addr& gateway,
                 const IPv4Addr& subnet,
                 const IPv4Addr& subnet_mask) {
  return new OSPFGateway(neighbor, gateway, subnet, subnet_mask);
}

OSPFGateway::OSPFGateway(Fwk::Ptr<OSPFNode> neighbor,
                         const IPv4Addr& gateway,
                         const IPv4Addr& subnet,
                         const IPv4Addr& subnet_mask)
    : OSPFLink::OSPFLink(neighbor, subnet, subnet_mask),
      gateway_(gateway),
      last_hello_(time(NULL)) {}

const OSPFInterface*
OSPFGateway::interface() const {
  return iface_;
}

OSPFInterface*
OSPFGateway::interface() {
  return iface_;
}

void
OSPFGateway::interfaceIs(OSPFInterface* iface) {
  iface_ = iface;
}

bool
OSPFGateway::operator==(const OSPFGateway& other) const {
  const OSPFLink* self_link = static_cast<const OSPFLink*>(this);
  const OSPFLink* other_link = static_cast<const OSPFLink*>(&other);
  if (*self_link != *other_link)
    return false;

  if (this->gateway() != other.gateway())
    return false;

  return true;
}

bool
OSPFGateway::operator!=(const OSPFGateway& other) const {
  return !(other == *this);
}
