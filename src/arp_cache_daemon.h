#ifndef ARP_CACHE_DAEMON_H_
#define ARP_CACHE_DAEMON_H_

#include "fwk/log.h"
#include "fwk/ptr_interface.h"

#include "arp_cache.h"
#include "task.h"


class ARPCacheDaemon : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const ARPCacheDaemon> PtrConst;
  typedef Fwk::Ptr<ARPCacheDaemon> Ptr;

  static Ptr New(ARPCache::Ptr cache) { return new ARPCacheDaemon(cache); }

 protected:
  ARPCacheDaemon(ARPCache::Ptr cache);

  void run();

  ARPCache::Ptr cache_;
  Fwk::Log::Ptr log_;
};


#endif
