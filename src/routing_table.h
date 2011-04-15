#ifndef ROUTING_TABLE_H_HE9H7VS9
#define ROUTING_TABLE_H_HE9H7VS9

#include <pthread.h>
#include <set>

#include "fwk/ptr_interface.h"
#include "fwk/linked_list.h"

#include "ip_packet.h"
#include "interface.h"


/* Thread safety: in a threaded environment, methods of this class must be
   accessed with lockedIs(true) or by using the ScopedLock. */
class RoutingTable : public Fwk::PtrInterface<RoutingTable> {
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

    static Ptr New() {
      return new Entry();
    }

    void operator=(const Entry& other);

    IPv4Addr subnet() const { return subnet_; }
    IPv4Addr subnetMask() const { return subnet_mask_; }
    IPv4Addr gateway() const { return gateway_; }
    Interface::Ptr interface() const { return interface_; }

    void subnetIs(const IPv4Addr& dest, const IPv4Addr& subnet_mask);
    void gatewayIs(const IPv4Addr& gateway) { gateway_ = gateway; }
    void interfaceIs(Interface::Ptr iface) { interface_ = iface; }

    Type type() const { return type_; }
    void typeIs(Type type) { type_ = type; }

   protected:
    Entry();

   private:
    IPv4Addr subnet_;
    IPv4Addr subnet_mask_;
    IPv4Addr gateway_;
    Interface::Ptr interface_;
    Type type_;

    /* Operations disallowed */
    Entry(const Entry&);
  };

  /* ScopedLock for safe locking of the routing table. */
  class ScopedLock {
   public:
    ScopedLock(RoutingTable::Ptr rtable) : rtable_(rtable) {
      rtable_->lockedIs(true);
    }
    ~ScopedLock() {
      rtable_->lockedIs(false);
      rtable_ = NULL;
    }

   private:
    RoutingTable::Ptr rtable_;
  };

  static Ptr New() {
    return new RoutingTable();
  }

  /* Locking for thread safety. */
  void lockedIs(bool locked);

  Entry::Ptr lpm(const IPv4Addr& dest_ip) const;
  Entry::Ptr front() const { return rtable_.front(); }

  void entryIs(Entry::Ptr entry);
  Entry::Ptr entryDel(Entry::Ptr entry) { return rtable_.del(entry); }

  /* Returns number of entries in routing table. */
  size_t entries() const { return rtable_.size(); }

 protected:
  RoutingTable();

 private:
  /* Routing table is a linked list. */
  Fwk::LinkedList<Entry> rtable_;

  /* Lock for modifications. */
  pthread_mutex_t lock_;

  /* Operations disallowed. */
  RoutingTable(const RoutingTable&);
  void operator=(const RoutingTable&);
};

#endif
