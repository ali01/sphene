#ifndef ROUTING_TABLE_H_HE9H7VS9
#define ROUTING_TABLE_H_HE9H7VS9

#include <set>

#include "fwk/ptr_interface.h"
#include "fwk/linked_list.h"

#include "ip_packet.h"
#include "interface.h"


class RoutingTable : public Fwk::PtrInterface<RoutingTable> {
 public:
  typedef Fwk::Ptr<const RoutingTable> PtrConst;
  typedef Fwk::Ptr<RoutingTable> Ptr;

  /* Nested routing table entry class */
  class Entry : public Fwk::LinkedList<Entry>::Node {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    static Ptr New() {
      return new Entry();
    }

    IPv4Addr subnet() const { return subnet_; }
    IPv4Addr subnetMask() const { return subnet_mask_; }
    IPv4Addr gateway() const { return gateway_; }
    Interface::Ptr interface() const { return interface_; }

    void subnetIs(const IPv4Addr& dest, const IPv4Addr& subnet_mask);
    void gatewayIs(const IPv4Addr& gateway) { gateway_ = gateway; }
    void interfaceIs(Interface::Ptr iface) { interface_ = iface; }

   protected:
    Entry();

   private:
    IPv4Addr subnet_;
    IPv4Addr subnet_mask_;
    IPv4Addr gateway_;
    Interface::Ptr interface_;

    /* Operations disallowed */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  static Ptr New() {
    return new RoutingTable();
  }

  Entry::Ptr lpm(const IPv4Addr& dest_ip) const;

  void entryIs(Entry::Ptr entry) { rtable_.pushFront(entry); }
  void entryDel(Entry::Ptr entry) { rtable_.del(entry); }

 protected:
  RoutingTable() {}

 private:
  /* Routing table is a linked list. */
  Fwk::LinkedList<Entry> rtable_;

  /* Operations disallowed. */
  RoutingTable(const RoutingTable&);
  void operator=(const RoutingTable&);
};

#endif
