#include "arp_cache_daemon.h"

#include <vector>

#include "arp_cache.h"
#include "fwk/log.h"
#include "task.h"
#include "time_types.h"

static const Seconds kTimeoutAge = 30;

using std::vector;


ARPCacheDaemon::ARPCacheDaemon(ARPCache::Ptr cache)
    : PeriodicTask("ARPCacheDaemon"),
      cache_(cache),
      log_(Fwk::Log::LogNew("ARPCacheDaemon")) { }


void ARPCacheDaemon::run() {
  vector<ARPCache::Entry::Ptr> remove;

  // Find old entries to remove.
  ARPCache::ScopedLock lock(cache_);
  for (ARPCache::iterator it = cache_->begin(); it != cache_->end(); ++it) {
    ARPCache::Entry::Ptr entry = it->second;

    if (entry->type() == ARPCache::Entry::kDynamic &&
        entry->age() >= kTimeoutAge.value()) {
      remove.push_back(entry);
    }
  }

  // Remove old entries.
  vector<ARPCache::Entry::Ptr>::iterator it;
  for (it = remove.begin(); it != remove.end(); ++it) {
    DLOG << "removing entry for " << (*it)->ipAddr();
    cache_->entryDel(*it);
  }
}
