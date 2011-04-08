#ifndef ARP_CACHE_H_JWDQ7ZZK
#define ARP_CACHE_H_JWDQ7ZZK

#include <ctime>
#include <map>

#include "fwk/ptr_interface.h"

#include "ethernet_packet.h"
#include "ip_packet.h"

class ARPCache : public Fwk::PtrInterface<ARPCache> {
 public:
  typedef Fwk::Ptr<const ARPCache> PtrConst;
  typedef Fwk::Ptr<ARPCache> Ptr;

  /* Nested cache entry class. */
  class Entry : public Fwk::PtrInterface<Entry> {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    static Ptr EntryNew(const IPv4Addr& ip, const EthernetAddr& eth) {
      return new Entry(ip, eth);
    }

    EthernetAddr ethernetAddr() const { return eth_; }
    IPv4Addr ipAddr() const { return ip_; }

    time_t age() const { return time(NULL) - t_; }
    void ageIs(time_t age) { t_ = time(NULL) - age; }

   protected:
    Entry(const IPv4Addr& ip, const EthernetAddr& eth)
        : ip_(ip), eth_(eth), t_(time(NULL)) {}

   private:
    /* Data members */
    const IPv4Addr ip_;
    const EthernetAddr eth_;
    time_t t_;

    /* Disallowed operations */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  typedef std::map<IPv4Addr,Entry::Ptr>::iterator iterator;
  typedef std::map<IPv4Addr,Entry::Ptr>::const_iterator const_iterator;

  static Ptr ARPCacheNew() {
    return new ARPCache();
  }

  size_t entries() const { return addr_map_.size(); }

  Entry::Ptr entry(const IPv4Addr& ip) const;
  void entryIs(Entry::Ptr entry);

  iterator begin() { return addr_map_.begin(); }
  iterator end() { return addr_map_.end(); }
  const_iterator begin() const { return addr_map_.begin(); }
  const_iterator end() const { return addr_map_.end(); }

 protected:
  ARPCache() {}

 private:
  /* Data members */
  std::map<IPv4Addr,Entry::Ptr> addr_map_;

  /* Disallowed operations. */
  ARPCache(const ARPCache&);
  void operator=(const ARPCache&);
};

#endif