#ifndef OSPF_NEIGHBOR_MAP_H_596RSZ34
#define OSPF_NEIGHBOR_MAP_H_596RSZ34

#include <map>

#include "fwk/linked_list.h"
#include "fwk/ptr_interface.h"

#include "interface.h"
#include "ipv4_addr.h"
#include "ospf_interface_desc.h"
#include "ospf_neighbor.h"


class OSPFNeighborMap : public Fwk::PtrInterface<OSPFNeighborMap> {
 public:
  typedef Fwk::Ptr<const OSPFNeighborMap> PtrConst;
  typedef Fwk::Ptr<OSPFNeighborMap> Ptr;

  typedef std::map<IPv4Addr,OSPFInterfaceDesc::Ptr>::iterator iterator;
  typedef std::map<IPv4Addr,OSPFInterfaceDesc::Ptr>::const_iterator
    const_iterator;

  /* Public constructor allows compile-time allocation. */
  OSPFNeighborMap() {}

  static Ptr OSPFNeighborMapNew() {
    return new OSPFNeighborMap();
  }

  OSPFInterfaceDesc::PtrConst interfaceDesc(const IPv4Addr& addr) const;
  OSPFInterfaceDesc::Ptr interfaceDesc(const IPv4Addr& addr);

  void interfaceDescIs(OSPFInterfaceDesc::Ptr iface_desc);
  void interfaceDescDel(OSPFInterfaceDesc::Ptr iface_desc);
  void interfaceDescDel(const IPv4Addr& addr);

  iterator begin() { return interfaces_.begin(); }
  iterator end() { return interfaces_.end(); }
  const_iterator begin() const { return interfaces_.begin(); }
  const_iterator end() const { return interfaces_.end(); }

 private:
  /* Data members. */
  std::map<IPv4Addr,OSPFInterfaceDesc::Ptr> interfaces_;

  /* Operations disallowed. */
  OSPFNeighborMap(const OSPFNeighborMap&);
  void operator=(const OSPFNeighborMap&);
};

#endif
