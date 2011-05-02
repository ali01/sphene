#include "ospf_neighbor.h"

#include "ospf_node.h"

OSPFNeighbor::Ptr
OSPFNeighbor::New(OSPFNode::Ptr neighbor,
                  const IPv4Addr& subnet,
                  const IPv4Addr& subnet_mask) {
  return new OSPFNeighbor(neighbor, subnet, subnet_mask);
}

OSPFNeighbor::Ptr
OSPFNeighbor::New(Fwk::Ptr<OSPFNode> neighbor) {
  return new OSPFNeighbor(neighbor, IPv4Addr::kZero, IPv4Addr::kMax);
}

OSPFNeighbor::OSPFNeighbor(OSPFNode::Ptr neighbor,
         const IPv4Addr& subnet,
         const IPv4Addr& subnet_mask)
    : node_(neighbor), subnet_(subnet), subnet_mask_(subnet_mask) {}

OSPFNode::PtrConst
OSPFNeighbor::node() const {
  return node_;
}

OSPFNode::Ptr
OSPFNeighbor::node() {
  return node_;
}
