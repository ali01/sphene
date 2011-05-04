#include "sw_data_plane.h"

#include "fwk/log.h"

#include "ethernet_packet.h"
#include "ip_packet.h"
#include "sr_integration.h"

struct sr_instance;


SWDataPlane::SWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("SWDataPlane", sr, routing_table, arp_cache),
      log_(Fwk::Log::LogNew("SWDataPlane")) { }
