#include "ospf_node.h"

OSPFNode::OSPFNode(uint32_t router_id, IPv4Addr subnet, IPv4Addr subnet_mask)
    : router_id_(router_id),
      subnet_(subnet),
      subnet_mask_(subnet_mask),
      last_refreshed_(time(NULL)),
      latest_seqno_(0) {}

OSPFNode::Ptr
OSPFNode::neighbor(uint32_t id) {
  return neighbors_.elem(id);
}

OSPFNode::PtrConst
OSPFNode::neighbor(uint32_t id) const {
  OSPFNode* self = const_cast<OSPFNode*>(this);
  return self->neighbor(id);
}

void
OSPFNode::neighborIs(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  neighbors_[node->routerID()] = node;
  node->neighborIs(this);
}

void
OSPFNode::neighborDel(uint32_t id) {
  OSPFNode::Ptr node = neighbor(id);
  node->neighborDel(this);
  neighbors_.elemDel(id);
}

void
OSPFNode::neighborDel(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  node->neighborDel(this);
  neighbors_.elemDel(node->routerID());
}
