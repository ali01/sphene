#include "arp_cache.h"

ARPCache::Entry::Ptr
ARPCache::entry(const IPv4Addr& ip) const {
  Entry::Ptr entry = NULL;
  const_iterator it = addr_map_.find(ip);
  if (it != this->end())
    entry = it->second;

  return entry;
}

void
ARPCache::entryIs(const IPv4Addr& ip, Entry::Ptr entry) {
  if (entry) {
    entry->ipAddrIs(ip);
    this->entryIs(entry);
  } else {
    addr_map_.erase(ip);
  }
}

void
ARPCache::entryIs(Entry::Ptr entry) {
  if (entry) {
    IPv4Addr key = entry->ipAddr();
    addr_map_[key] = entry;
  }
}
