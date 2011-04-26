#ifndef OSPF_INTERFACE_DESC_H_6WPUE39N
#define OSPF_INTERFACE_DESC_H_6WPUE39N

#include <map>

#include "fwk/ptr_interface.h"

#include "interface.h"
#include "ospf_neighbor.h"


class OSPFInterfaceDesc : public Fwk::PtrInterface<OSPFInterfaceDesc> {
 public:
  typedef Fwk::Ptr<const OSPFInterfaceDesc> PtrConst;
  typedef Fwk::Ptr<OSPFInterfaceDesc> Ptr;

  typedef std::map<uint32_t,OSPFNeighbor::PtrConst>::iterator iterator;
  typedef std::map<uint32_t,OSPFNeighbor::PtrConst>::const_iterator
    const_iterator;

  static Ptr New(Interface::Ptr iface, uint16_t helloint) {
    return new OSPFInterfaceDesc(iface, helloint);
  }

  Interface::PtrConst interface() const { return iface_; }
  Interface::Ptr interface() { return iface_; }

  uint16_t helloint() const { return helloint_; }

  OSPFNeighbor::PtrConst neighbor(uint32_t router_id) const;

  void neighborIs(OSPFNeighbor::PtrConst nb);
  void neighborDel(OSPFNeighbor::PtrConst nb);
  void neighborDel(uint32_t router_id);

  iterator begin() { return neighbors_.begin(); }
  iterator end() { return neighbors_.end(); }
  const_iterator begin() const { return neighbors_.begin(); }
  const_iterator end() const { return neighbors_.end(); }

 private:
  OSPFInterfaceDesc(Interface::Ptr iface, uint16_t helloint);

  /* Data members. */
  Interface::Ptr iface_;
  uint16_t helloint_;
  std::map<uint32_t,OSPFNeighbor::PtrConst> neighbors_;

  /* Operations disallowed. */
  OSPFInterfaceDesc(const OSPFInterfaceDesc&);
  void operator=(const OSPFInterfaceDesc&);
};

#endif
