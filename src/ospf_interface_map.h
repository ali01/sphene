#ifndef OSPF_NEIGHBOR_MAP_H_596RSZ34
#define OSPF_NEIGHBOR_MAP_H_596RSZ34

#include "fwk/linked_list.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_interface_desc.h"


class OSPFInterfaceMap : public Fwk::PtrInterface<OSPFInterfaceMap> {
 public:
  typedef Fwk::Ptr<const OSPFInterfaceMap> PtrConst;
  typedef Fwk::Ptr<OSPFInterfaceMap> Ptr;

  typedef Fwk::Map<IPv4Addr,OSPFInterfaceDesc>::iterator iterator;
  typedef Fwk::Map<IPv4Addr,OSPFInterfaceDesc>::const_iterator
    const_iterator;

  static Ptr New() {
    return new OSPFInterfaceMap();
  }

  OSPFInterfaceDesc::PtrConst interface(const IPv4Addr& addr) const;
  OSPFInterfaceDesc::Ptr interface(const IPv4Addr& addr);

  void interfaceIs(OSPFInterfaceDesc::Ptr iface_desc);
  void interfaceDel(OSPFInterfaceDesc::Ptr iface_desc);
  void interfaceDel(const IPv4Addr& addr);

  iterator begin() { return interfaces_.begin(); }
  iterator end() { return interfaces_.end(); }
  const_iterator begin() const { return interfaces_.begin(); }
  const_iterator end() const { return interfaces_.end(); }

 protected:
  OSPFInterfaceMap() {}

 private:
  /* Data members. */
  Fwk::Map<IPv4Addr,OSPFInterfaceDesc> interfaces_;

  /* Operations disallowed. */
  OSPFInterfaceMap(const OSPFInterfaceMap&);
  void operator=(const OSPFInterfaceMap&);
};

#endif
