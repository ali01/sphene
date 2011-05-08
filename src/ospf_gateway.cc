#include "ospf_gateway.h"

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
