#include "hw_data_plane.h"

#include "fwk/log.h"

#include "arp_cache.h"
#include "ethernet_packet.h"
#include "ip_packet.h"
#include "routing_table.h"

struct sr_instance;


HWDataPlane::HWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("HWDataPlane", sr, routing_table, arp_cache),
      arp_cache_reactor_(ARPCacheReactor(this)),
      log_(Fwk::Log::LogNew("HWDataPlane")) {
  arp_cache_reactor_.notifierIs(arp_cache);
}


HWDataPlane::ARPCacheReactor::ARPCacheReactor(HWDataPlane* dp)
    : dp_(dp),
      log_(Fwk::Log::LogNew("HWDataPlane::ARPCacheReactor")) { }


void HWDataPlane::ARPCacheReactor::onEntry(ARPCache::Entry::Ptr entry) {
  // TODO(ms): Write table to hardware.
}


void HWDataPlane::ARPCacheReactor::onEntryDel(ARPCache::Entry::Ptr entry) {
  // TODO(ms): Write table to hardware.
}
