#ifndef OSPF_INTERFACE_DESC_H_6WPUE39N
#define OSPF_INTERFACE_DESC_H_6WPUE39N

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_neighbor.h"
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

  /* Accessors. */

  Fwk::Ptr<const Interface> interface() const;
  Seconds helloint() const { return helloint_; }

  OSPFNode::Ptr neighbor(const RouterID& router_id);
  OSPFNode::PtrConst neighbor(const RouterID& router_id) const;

  IPv4Addr neighborSubnet(const RouterID& router_id) const;
  IPv4Addr neighborSubnetMask(const RouterID& router_id) const;

  /* Mutators. */

  void neighborIs(OSPFNode::Ptr nb,
                  const IPv4Addr& subnet,
                  const IPv4Addr& subnet_mask);
  void neighborDel(OSPFNode::Ptr nb);
  void neighborDel(const RouterID& router_id);

  /* Iterators. */

  iterator neighborsBegin() { return neighbor_nodes_.begin(); }
  iterator neighborsEnd() { return neighbor_nodes_.end(); }
  const_iterator neighborsBegin() const { return neighbor_nodes_.begin(); }
  const_iterator neighborsEnd() const { return neighbor_nodes_.end(); }

 private:
  OSPFInterfaceDesc(Fwk::Ptr<const Interface> iface, uint16_t helloint);

  /* Data members. */
  Fwk::Ptr<const Interface> iface_;
  uint16_t helloint_;

  /* Map of all neighbors directly attached to this router. */
  Fwk::Map<RouterID,OSPFNeighbor> neighbors_;

  /* Mirror map with direct pointers to neighboring OSPFNodes (rather than
     OSPFNeighbor objects). Used to provide iterators. If space constraints
     become a problem, this can be optimized away by defining custom
     iterators. */
  Fwk::Map<RouterID,OSPFNode> neighbor_nodes_;

  /* Operations disallowed. */
  OSPFInterfaceDesc(const OSPFInterfaceDesc&);
  void operator=(const OSPFInterfaceDesc&);
};

#endif
