#include "arp_cache.h"

#include <ctime>
#include <pthread.h>

#include "fwk/notifier.h"

const size_t ARPCache::kMaxEntries = 32;


ARPCache::ARPCache()
    : Fwk::BaseNotifier<ARPCache, ARPCacheNotifiee>("ARPCache") { }


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
  if (entry == NULL || addr_map_.find(entry->ipAddr()) != end())
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

  // Dispatch notification.
  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onEntry(this, entry);
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
  if (addr_map_.find(ip) == end())
    return;
  Entry::Ptr entry = addr_map_[ip];

  addr_map_.erase(ip);

  // Dispatch notification.
  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onEntryDel(this, entry);
}
