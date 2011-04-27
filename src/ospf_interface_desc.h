#ifndef OSPF_INTERFACE_DESC_H_6WPUE39N
#define OSPF_INTERFACE_DESC_H_6WPUE39N

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "interface.h"
#include "ospf_neighbor.h"
#include "time_types.h"


class OSPFInterfaceDesc : public Fwk::PtrInterface<OSPFInterfaceDesc> {
 public:
  typedef Fwk::Ptr<const OSPFInterfaceDesc> PtrConst;
  typedef Fwk::Ptr<OSPFInterfaceDesc> Ptr;

  typedef Fwk::Map<uint32_t,OSPFNeighbor>::iterator iterator;
  typedef Fwk::Map<uint32_t,OSPFNeighbor>::const_iterator
    const_iterator;

  static Ptr New(Interface::PtrConst iface, uint16_t helloint) {
    return new OSPFInterfaceDesc(iface, helloint);
  }

  Interface::PtrConst interface() const { return iface_; }
  Seconds helloint() const { return helloint_; }

  OSPFNeighbor::Ptr neighbor(uint32_t router_id);
  OSPFNeighbor::PtrConst neighbor(uint32_t router_id) const;

  void neighborIs(OSPFNeighbor::Ptr nb);
  void neighborDel(OSPFNeighbor::Ptr nb);
  void neighborDel(uint32_t router_id);

  iterator begin() { return neighbors_.begin(); }
  iterator end() { return neighbors_.end(); }
  const_iterator begin() const { return neighbors_.begin(); }
  const_iterator end() const { return neighbors_.end(); }

 private:
  OSPFInterfaceDesc(Interface::PtrConst iface, uint16_t helloint);

  /* Data members. */
  Interface::PtrConst iface_;
  uint16_t helloint_;
  Fwk::Map<uint32_t,OSPFNeighbor> neighbors_;

  /* Operations disallowed. */
  OSPFInterfaceDesc(const OSPFInterfaceDesc&);
  void operator=(const OSPFInterfaceDesc&);
};

#endif