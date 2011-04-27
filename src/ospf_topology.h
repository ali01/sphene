#ifndef OSPF_TOPOLOGY_H_I3MK5NYZ
#define OSPF_TOPOLOGY_H_I3MK5NYZ



#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ospf_node.h"


class OSPFTopology : public Fwk::PtrInterface<OSPFTopology> {
 public:
  typedef Fwk::Ptr<const OSPFTopology> PtrConst;
  typedef Fwk::Ptr<OSPFTopology> Ptr;

  typedef Fwk::Map<uint32_t,OSPFNode>::iterator iterator;
  typedef Fwk::Map<uint32_t,OSPFNode>::const_iterator const_iterator;

  static Ptr New() {
    return new OSPFTopology();
  }

  OSPFNode::Ptr node(uint32_t router_id);
  OSPFNode::PtrConst node(uint32_t router_id) const;

  void nodeIs(OSPFNode::Ptr node);
  void nodeDel(OSPFNode::Ptr node);
  void nodeDel(uint32_t router_id);

  iterator nodesBegin() { return nodes_.begin(); }
  iterator nodesEnd() { return nodes_.end(); }
  const_iterator nodesBegin() const { return nodes_.begin(); }
  const_iterator nodesEnd() const { return nodes_.end(); }

 private:
  OSPFTopology();

  /* Data members. */
  Fwk::Map<uint32_t,OSPFNode> nodes_;

  /* Operations disallowed. */
  OSPFTopology(const OSPFTopology&);
  void operator=(const OSPFTopology&);
};

#endif
