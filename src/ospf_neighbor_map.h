#ifndef OSPF_NEIGHBOR_MAP_H_596RSZ34
#define OSPF_NEIGHBOR_MAP_H_596RSZ34

#include <map>

#include "fwk/linked_list.h"
#include "fwk/ptr_interface.h"

#include "interface.h"
#include "ipv4_addr.h"

class OSPFNeighborMap : public Fwk::PtrInterface<OSPFNeighborMap> {
 public:
  typedef Fwk::Ptr<const OSPFNeighborMap> PtrConst;
  typedef Fwk::Ptr<OSPFNeighborMap> Ptr;

  class Neighbor : public Fwk::LinkedList<Neighbor>::Node {
   public:
    typedef Fwk::Ptr<const Neighbor> PtrConst;
    typedef Fwk::Ptr<Neighbor> Ptr;

    /* Expects and returns all values in host byte-order. */
    static Ptr New(uint32_t id, IPv4Addr iface_addr) {
      return new Neighbor(id, iface_addr);
    }

    uint32_t routerId() const { return id_; }
    IPv4Addr interfaceAddr() const { return iface_addr_; }

   private:
    Neighbor(uint32_t id, IPv4Addr iface_addr);

    /* Data members. */
    uint32_t id_;
    IPv4Addr iface_addr_;

    /* Operations disallowed. */
    Neighbor(const Neighbor&);
    void operator=(const Neighbor&);
  };


  class InterfaceDesc : public Fwk::PtrInterface<InterfaceDesc> {
   public:
    typedef Fwk::Ptr<const InterfaceDesc> PtrConst;
    typedef Fwk::Ptr<InterfaceDesc> Ptr;

    static Ptr New(Interface::Ptr iface, uint16_t helloint) {
      return new InterfaceDesc(iface, helloint);
    }

    Interface::PtrConst interface() const { return iface_; }
    Interface::Ptr interface() { return iface_; }

    uint16_t helloint() const { return helloint_; }

    Neighbor::PtrConst neighborFront() const { return neighbor_list_.front(); }
    void neighborIs(Neighbor::Ptr _n) { neighbor_list_.pushBack(_n); }

   private:
    InterfaceDesc(Interface::Ptr iface, uint16_t helloint);

    /* Data members. */
    Interface::Ptr iface_;
    uint16_t helloint_;
    Fwk::LinkedList<Neighbor> neighbor_list_;

    /* Operations disallowed. */
    InterfaceDesc(const InterfaceDesc&);
    void operator=(const InterfaceDesc&);
  };

  typedef std::map<IPv4Addr,InterfaceDesc::Ptr>::iterator iterator;
  typedef std::map<IPv4Addr,InterfaceDesc::Ptr>::const_iterator const_iterator;

  /* Public constructor allows compile-time allocation. */
  OSPFNeighborMap() {}

  static Ptr OSPFNeighborMapNew() {
    return new OSPFNeighborMap();
  }

  InterfaceDesc::PtrConst interfaceDesc(const IPv4Addr& addr) const;
  InterfaceDesc::Ptr interfaceDesc(const IPv4Addr& addr);

  void interfaceDescIs(InterfaceDesc::Ptr iface_desc);
  void interfaceDescDel(InterfaceDesc::Ptr iface_desc);
  void interfaceDescDel(const IPv4Addr& addr);

  iterator begin() { return interfaces_.begin(); }
  iterator end() { return interfaces_.end(); }
  const_iterator begin() const { return interfaces_.begin(); }
  const_iterator end() const { return interfaces_.end(); }

 private:
  /* Data members. */
  std::map<IPv4Addr,InterfaceDesc::Ptr> interfaces_;

  /* Operations disallowed. */
  OSPFNeighborMap(const OSPFNeighborMap&);
  void operator=(const OSPFNeighborMap&);
};

#endif
