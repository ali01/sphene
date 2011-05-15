#ifndef ROUTING_TABLE_H_HE9H7VS9
#define ROUTING_TABLE_H_HE9H7VS9

#include "fwk/locked_interface.h"
#include "fwk/notifier.h"
#include "fwk/map.h"

#include "interface_map.h"
#include "ipv4_addr.h"


/* Forward declarations. */
class Interface;
class RoutingTableNotifiee;

/* Thread safety: in a threaded environment, methods of this class must be
   accessed with lockedIs(true) or by using the ScopedLock. */
class RoutingTable
    : public Fwk::BaseNotifier<RoutingTable, RoutingTableNotifiee>,
      public Fwk::LockedInterface {
 public:
  typedef Fwk::Ptr<const RoutingTable> PtrConst;
  typedef Fwk::Ptr<RoutingTable> Ptr;
  typedef RoutingTableNotifiee Notifiee;
  typedef std::pair<IPv4Addr,IPv4Addr> Subnet;

  /* Nested routing table entry class */
  class Entry : public Fwk::PtrInterface<Entry> {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    enum Type {
      kDynamic,
      kStatic
    };

    static Ptr New(Type type=kDynamic) {
      return new Entry(type);
    }

    /* RoutingTable supports entry lookup by subnet. To make this safe,
       the interface of Entry enforces the following equality:

         subnet == subnet & subnet_mask

       For this reason, two entries with equivalent subnets also have
       equivalent subnet_masks.
     */

    IPv4Addr subnet() const { return subnet_; }
    IPv4Addr subnetMask() const { return subnet_mask_; }
    IPv4Addr gateway() const { return gateway_; }
    Fwk::Ptr<const Interface> interface() const;
    Type type() const { return type_; }

    void subnetIs(const IPv4Addr& dest, const IPv4Addr& subnet_mask);
    void gatewayIs(const IPv4Addr& gateway) { gateway_ = gateway; }
    void interfaceIs(Fwk::Ptr<const Interface> iface);

    /* Comparison operators. */
    bool operator==(const Entry&) const;
    bool operator!=(const Entry&) const;

   protected:
    Entry(Type type);

   private:
    IPv4Addr subnet_;
    IPv4Addr subnet_mask_;
    IPv4Addr gateway_;
    Fwk::Ptr<const Interface> interface_;
    Type type_;

    /* Operations disallowed */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  /* Iterator types. */
  typedef Fwk::Map<Subnet,Entry>::const_iterator const_iterator;
  typedef Fwk::Map<Subnet,Entry>::iterator iterator;

  static Ptr New(InterfaceMap::Ptr iface_map) {
    return new RoutingTable(iface_map);
  }

  /* Accessors. */

  Entry::Ptr entry(const IPv4Addr& subnet, const IPv4Addr& mask);
  Entry::PtrConst entry(const IPv4Addr& subnet, const IPv4Addr& mask) const;

  Entry::Ptr lpm(const IPv4Addr& dest_ip);
  Entry::PtrConst lpm(const IPv4Addr& dest_ip) const;

  size_t entries() const { return rtable_.size(); }

  /* Mutators. */

  void entryIs(Entry::Ptr entry);
  void entryDel(Entry::Ptr entry);
  void entryDel(const IPv4Addr& subnet, const IPv4Addr& mask);

  /* Calls entryDel on all entries of type
     Entry::kDynamic in the routing table. */
  void clearDynamicEntries();

  /* Iterators. */
  iterator entriesBegin() { return rtable_.begin(); }
  iterator entriesEnd() { return rtable_.end(); }
  const_iterator entriesBegin() const { return rtable_.begin(); }
  const_iterator entriesEnd() const { return rtable_.end(); }

 protected:
  RoutingTable(InterfaceMap::Ptr iface_map);

 private:
  class InterfaceMapReactor : public InterfaceMap::Notifiee {
   public:
    typedef Fwk::Ptr<const InterfaceMapReactor> PtrConst;
    typedef Fwk::Ptr<InterfaceMapReactor> Ptr;

    static Ptr New(RoutingTable* _rt) {
      return new InterfaceMapReactor(_rt);
    }

    void onInterface(InterfaceMap::Ptr map, Interface::Ptr iface);
    void onInterfaceDel(InterfaceMap::Ptr map, Interface::Ptr iface);

   private:
    InterfaceMapReactor(RoutingTable* _rt) : rtable_(_rt) {}

    /* Data members. */
    RoutingTable* rtable_;

    /* Operations disallowed. */
    InterfaceMapReactor(const InterfaceMapReactor&);
    void operator=(const InterfaceMapReactor&);
  };

  /* Data members. */
  Fwk::Map<Subnet,Entry> rtable_;
  Fwk::Map<Subnet,Entry> rtable_dynamic_;
  Fwk::Map<Subnet,Entry> rtable_static_;

  InterfaceMapReactor::Ptr iface_map_reactor_;

  /* Operations disallowed. */
  RoutingTable(const RoutingTable&);
  void operator=(const RoutingTable&);
};


class RoutingTableNotifiee
    : public Fwk::BaseNotifiee<RoutingTable, RoutingTableNotifiee> {
 public:
  virtual void onEntry(RoutingTable::Ptr rtable,
                       RoutingTable::Entry::Ptr entry) { }
  virtual void onEntryDel(RoutingTable::Ptr rtable,
                          RoutingTable::Entry::Ptr entry) { }
};

#endif
