#include "arp_cache.h"

#include <ctime>
#include <pthread.h>

const size_t ARPCache::kMaxEntries = 32;


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
  if (entry == NULL)
    return;

  if (entries() >= kMaxEntries) {
    // Evict oldest entry.
    time_t o_age = 0;
    Entry::Ptr o_entry = NULL;
    for (iterator it = begin(); it != end(); ++it) {
      Entry::Ptr entry = it->second;
      if (entry->age() > o_age) {
        o_age = entry->age();
        o_entry = entry;
      }
    }
    entryDel(o_entry);
  }

  IPv4Addr key = entry->ipAddr();
  addr_map_[key] = entry;
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
