#ifndef ARP_CACHE_H_JWDQ7ZZK
#define ARP_CACHE_H_JWDQ7ZZK

#include <ctime>
#include <map>

#include "fwk/locked_interface.h"
#include "fwk/notifier.h"
#include "fwk/ptr.h"

#include "ethernet_packet.h"
#include "ip_packet.h"


class ARPCacheNotifiee;

/* Thread safety: in a threaded environment, methods of this class must be
   accessed with lockedIs(true) or by using the ScopedLock. */
class ARPCache : public Fwk::LockedInterface,
                 public Fwk::BaseNotifier<ARPCache, ARPCacheNotifiee> {
 public:
  typedef Fwk::Ptr<const ARPCache> PtrConst;
  typedef Fwk::Ptr<ARPCache> Ptr;
  typedef ARPCacheNotifiee Notifiee;

  /* Nested cache entry class. */
  class Entry : public Fwk::PtrInterface<Entry> {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    enum Type {
      kDynamic,
      kStatic
    };

    static Ptr New(const IPv4Addr& ip, const EthernetAddr& eth) {
      return new Entry(ip, eth);
    }

    const EthernetAddr& ethernetAddr() const { return eth_; }
    const IPv4Addr& ipAddr() const { return ip_; }

    void ethernetAddrIs(const EthernetAddr& eth) { eth_ = eth; }
    void ipAddrIs(const IPv4Addr& ip) { ip_ = ip; }

    time_t age() const { return time(NULL) - t_; }
    void ageIs(time_t age) { t_ = time(NULL) - age; }

    Type type() const { return type_; }
    void typeIs(Type type) { type_ = type; }

   protected:
    Entry(const IPv4Addr& ip, const EthernetAddr& eth)
        : ip_(ip), eth_(eth), t_(time(NULL)), type_(kDynamic) { }

   private:
    /* Data members */
    IPv4Addr ip_;
    EthernetAddr eth_;
    time_t t_;
    Type type_;

    /* Disallowed operations */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  typedef std::map<IPv4Addr, Entry::Ptr>::iterator iterator;
  typedef std::map<IPv4Addr, Entry::Ptr>::const_iterator const_iterator;

  /* Maximum number of entries the cache will hold. */
  static const size_t kMaxEntries;

  static Ptr New() {
    return new ARPCache();
  }

  size_t entries() const { return addr_map_.size(); }

  Entry::Ptr entry(const IPv4Addr& ip) const;
  void entryIs(Entry::Ptr entry);
  void entryDel(const IPv4Addr& ip);
  void entryDel(Entry::Ptr entry);

  iterator begin() { return addr_map_.begin(); }
  iterator end() { return addr_map_.end(); }
  const_iterator begin() const { return addr_map_.begin(); }
  const_iterator end() const { return addr_map_.end(); }

 protected:
  ARPCache();

 private:
  /* Data members */
  std::map<IPv4Addr,Entry::Ptr> addr_map_;

  /* Disallowed operations. */
  ARPCache(const ARPCache&);
  void operator=(const ARPCache&);
};


class ARPCacheNotifiee : public Fwk::BaseNotifiee<ARPCache, ARPCacheNotifiee> {
 public:
  virtual void onEntry(ARPCache::Ptr cache, ARPCache::Entry::Ptr entry) { }
  virtual void onEntryDel(ARPCache::Ptr cache, ARPCache::Entry::Ptr entry) { }
};


#endif
