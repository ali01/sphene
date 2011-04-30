#include "arp_cache.h"

#include <pthread.h>


ARPCache::ARPCache() { }


ARPCache::Entry::Ptr
ARPCache::entry(const IPv4Addr& ip) const {
  Entry::Ptr entry = NULL;
  const_iterator it = addr_map_.find(ip);
  if (it != this->end())
    entry = it->second;

  return entry;
}

void
ARPCache::entryIs(Entry::Ptr entry) {
  if (entry) {
    IPv4Addr key = entry->ipAddr();
    addr_map_[key] = entry;
  }
}

void
ARPCache::entryDel(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  IPv4Addr key = entry->ipAddr();
  this->entryDel(key);
}

void
ARPCache::entryDel(const IPv4Addr& ip) {
  addr_map_.erase(ip);
}
