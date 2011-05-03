#ifndef OSPF_INTERFACE_DESC_H_6WPUE39N
#define OSPF_INTERFACE_DESC_H_6WPUE39N

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ospf_node.h"
#include "ospf_types.h"
#include "time_types.h"

/* Forward declarations. */
class Interface;


class OSPFInterfaceDesc : public Fwk::PtrInterface<OSPFInterfaceDesc> {
 public:
  typedef Fwk::Ptr<const OSPFInterfaceDesc> PtrConst;
  typedef Fwk::Ptr<OSPFInterfaceDesc> Ptr;

  typedef Fwk::Map<RouterID,OSPFNode>::iterator iterator;
  typedef Fwk::Map<RouterID,OSPFNode>::const_iterator
    const_iterator;

  static Ptr New(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  Fwk::Ptr<const Interface> interface() const;
  Seconds helloint() const { return helloint_; }

  OSPFNode::Ptr neighbor(const RouterID& router_id);
  OSPFNode::PtrConst neighbor(const RouterID& router_id) const;

  void neighborIs(OSPFNode::Ptr nb);
  void neighborDel(OSPFNode::Ptr nb);
  void neighborDel(const RouterID& router_id);

  iterator neighborsBegin() { return neighbors_.begin(); }
  iterator neighborsEnd() { return neighbors_.end(); }
  const_iterator neighborsBegin() const { return neighbors_.begin(); }
  const_iterator neighborsEnd() const { return neighbors_.end(); }

 private:
  OSPFInterfaceDesc(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  /* Data members. */
  Fwk::Ptr<const Interface> iface_;
  uint16_t helloint_;
  Fwk::Map<RouterID,OSPFNode> neighbors_;

  /* Operations disallowed. */
  OSPFInterfaceDesc(const OSPFInterfaceDesc&);
  void operator=(const OSPFInterfaceDesc&);
};

#endif
