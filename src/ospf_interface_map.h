#ifndef OSPF_NEIGHBOR_MAP_H_596RSZ34
#define OSPF_NEIGHBOR_MAP_H_596RSZ34

#include "fwk/linked_list.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_interface.h"
#include "ospf_types.h"


class OSPFInterfaceMap : public Fwk::PtrInterface<OSPFInterfaceMap> {
 public:
  typedef Fwk::Ptr<const OSPFInterfaceMap> PtrConst;
  typedef Fwk::Ptr<OSPFInterfaceMap> Ptr;

  typedef Fwk::Map<IPv4Addr,OSPFInterface>::iterator iterator;
  typedef Fwk::Map<IPv4Addr,OSPFInterface>::const_iterator
    const_iterator;

  static Ptr New() {
    return new OSPFInterfaceMap();
  }

  OSPFInterface::PtrConst interface(const IPv4Addr& addr) const;
  OSPFInterface::Ptr interface(const IPv4Addr& addr);

  void interfaceIs(OSPFInterface::Ptr iface_desc);
  void interfaceDel(OSPFInterface::Ptr iface_desc);
  void interfaceDel(const IPv4Addr& addr);

  iterator begin() { return interfaces_.begin(); }
  iterator end() { return interfaces_.end(); }
  const_iterator begin() const { return interfaces_.begin(); }
  const_iterator end() const { return interfaces_.end(); }

 protected:
  OSPFInterfaceMap();

 private:
  class InterfaceReactor : public OSPFInterface::Notifiee {
   public:
    typedef Fwk::Ptr<const InterfaceReactor> PtrConst;
    typedef Fwk::Ptr<InterfaceReactor> Ptr;

    static Ptr New(OSPFInterfaceMap::Ptr _im) {
      return new InterfaceReactor(_im);
    }

    void onNeighbor(OSPFInterface::Ptr iface, const RouterID& id) {}
    void onNeighborDel(OSPFInterface::Ptr iface, const RouterID& id) {}

   private:
    InterfaceReactor(OSPFInterfaceMap::Ptr _im) : iface_map_(_im.ptr()) {}

    /* Data members. */
    OSPFInterfaceMap* iface_map_; /* Weak ptr prevents circular reference. */

    /* Operations disallowed. */
    InterfaceReactor(const InterfaceReactor&);
    void operator=(const InterfaceReactor&);
  };

  /* Data members. */
  Fwk::Map<IPv4Addr,OSPFInterface> interfaces_;
  InterfaceReactor::Ptr iface_reactor_;

  /* Operations disallowed. */
  OSPFInterfaceMap(const OSPFInterfaceMap&);
  void operator=(const OSPFInterfaceMap&);
};

#endif
