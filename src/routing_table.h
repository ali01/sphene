#ifndef ROUTING_TABLE_H_HE9H7VS9
#define ROUTING_TABLE_H_HE9H7VS9

#include "fwk/ptr_interface.h"

#include "ip_packet.h"

class RoutingTable : public Fwk::PtrInterface<RoutingTable> {
 public:
  typedef Fwk::Ptr<const RoutingTable> PtrConst;
  typedef Fwk::Ptr<RoutingTable> Ptr;

  /* Nested routing table entry class */
  class Entry : public Fwk::PtrInterface<Entry> {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    static Ptr EntryNew() {
      return new Entry();
    }

   protected:
    Entry() {}

   private:
    /* Operations disallowed */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  static Ptr RoutingTableNew() {
    return new RoutingTable();
  }

  Entry::Ptr longestPrefixMatch(const IPv4Addr& dest_ip);

  void entryIs(Entry::Ptr entry);

 protected:
  RoutingTable() {}

 private:
  /* Operations disallowed. */
  RoutingTable(const RoutingTable&);
  void operator=(const RoutingTable&);
};

#endif
