#include "ospf_node.h"

OSPFNode::OSPFNode(const RouterID& router_id, IPv4Addr subnet)
    : router_id_(router_id),
      subnet_(subnet),
      subnet_mask_(IPv4Addr::kMax),
      last_refreshed_(time(NULL)),
      latest_seqno_(0),
      distance_(0) {}

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
OSPFNode::neighborIs(OSPFNode::Ptr node) {
  if (node == NULL)
    return;

  neighbors_[node->routerID()] = node;
  node->neighborIs(this);
}

void
OSPFNode::neighborDel(const RouterID& id) {
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
