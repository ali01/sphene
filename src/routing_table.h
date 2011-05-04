#ifndef ROUTING_TABLE_H_HE9H7VS9
#define ROUTING_TABLE_H_HE9H7VS9

#include <set>

#include "fwk/linked_list.h"
#include "fwk/locked_interface.h"
#include "fwk/ptr_interface.h"
#include "fwk/scoped_lock.h"

#include "ip_packet.h"
#include "interface.h"


/* Thread safety: in a threaded environment, methods of this class must be
   accessed with lockedIs(true) or by using the ScopedLock. */
class RoutingTable : public Fwk::PtrInterface<RoutingTable>,
                     public Fwk::LockedInterface {
 public:
  typedef Fwk::Ptr<const RoutingTable> PtrConst;
  typedef Fwk::Ptr<RoutingTable> Ptr;

  /* Nested routing table entry class */
  class Entry : public Fwk::LinkedList<Entry>::Node {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    enum Type {
      kDynamic,
      kStatic
    };

    static Ptr New(Type type) {
      return new Entry(type);
    }

    IPv4Addr subnet() const { return subnet_; }
    IPv4Addr subnetMask() const { return subnet_mask_; }
    IPv4Addr gateway() const { return gateway_; }
    Interface::Ptr interface() const { return interface_; }
    Type type() const { return type_; }

    void subnetIs(const IPv4Addr& dest, const IPv4Addr& subnet_mask);
    void gatewayIs(const IPv4Addr& gateway) { gateway_ = gateway; }
    void interfaceIs(Interface::Ptr iface) { interface_ = iface; }

   protected:
    Entry(Type type);

   private:
    IPv4Addr subnet_;
    IPv4Addr subnet_mask_;
    IPv4Addr gateway_;
    Interface::Ptr interface_;
    Type type_;

    /* Operations disallowed */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  static Ptr New() {
    return new RoutingTable();
  }

  Entry::Ptr lpm(const IPv4Addr& dest_ip);
  Entry::Ptr front() { return rtable_.front(); }

  Entry::PtrConst lpm(const IPv4Addr& dest_ip) const;
  Entry::PtrConst front() const { return rtable_.front(); }

  void entryIs(Entry::Ptr entry);
  Entry::Ptr entryDel(Entry::Ptr entry) { return rtable_.del(entry); }

  /* Returns number of entries in routing table. */
  size_t entries() const { return rtable_.size(); }

 protected:
  RoutingTable();

 private:
  /* Routing table is a linked list. */
  Fwk::LinkedList<Entry> rtable_;

  /* Operations disallowed. */
  RoutingTable(const RoutingTable&);
  void operator=(const RoutingTable&);
};

#endif
