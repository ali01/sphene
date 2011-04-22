#include "arp_cache_daemon.h"

#include "arp_cache.h"
#include "fwk/log.h"
#include "task.h"


ARPCacheDaemon::ARPCacheDaemon(ARPCache::Ptr cache)
    : PeriodicTask("ARPCacheDaemon"),
      cache_(cache),
      log_(Fwk::Log::LogNew("ARPCacheDaemon")) { }


void ARPCacheDaemon::run() {
  // TODO(ms): Implement this.
  DLOG << "ARP Cache Daemon running";
}
