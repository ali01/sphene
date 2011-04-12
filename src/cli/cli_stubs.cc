#include "cli_stubs.h"

#include <inttypes.h>
#include <stdio.h>
#include <vector>

#include "arp_cache.h"
#include "control_plane.h"
#include "sr_base_internal.h"

using std::vector;


int arp_cache_static_entry_add( struct sr_instance* sr,
                                uint32_t ip,
                                uint8_t* mac ) {
  ARPCache::Ptr cache = sr->cp->arpCache();
  ARPCache::Entry::Ptr cache_entry = ARPCache::Entry::EntryNew(ip, mac);
  cache_entry->typeIs(ARPCache::Entry::kStatic);
  cache->lockedIs(true);
  cache->entryIs(cache_entry);
  cache->lockedIs(false);

  // TODO(ms): Max number of entries.
  return 1;
}


int arp_cache_static_entry_remove( struct sr_instance* sr, uint32_t ip ) {
  ARPCache::Ptr cache = sr->cp->arpCache();
  cache->lockedIs(true);

  ARPCache::Entry::Ptr cache_entry = cache->entry(ip);
  if (!cache_entry || cache_entry->type() != ARPCache::Entry::kStatic) {
    cache->lockedIs(false);
    return 0;
  }

  cache->entryDel(cache_entry);
  cache->lockedIs(false);
  return 1;
}


// Removes all entries from the ARP Cache of the specified type.
static unsigned arp_cache_type_purge(struct sr_instance* sr,
                                     ARPCache::Entry::Type type) {
  ARPCache::Ptr cache = sr->cp->arpCache();
  cache->lockedIs(true);

  // Build list of entries to remove.
  vector<ARPCache::Entry::Ptr> entries;
  for (ARPCache::iterator it = cache->begin(); it != cache->end(); ++it) {
    ARPCache::Entry::Ptr entry = it->second;
    if (entry->type() == type)
      entries.push_back(entry);
  }

  // Remove the chosen entries.
  for (vector<ARPCache::Entry::Ptr>::iterator it = entries.begin();
       it != entries.end(); ++it) {
    cache->entryDel(*it);
  }

  return entries.size();
}


unsigned arp_cache_static_purge( struct sr_instance* sr ) {
  return arp_cache_type_purge(sr, ARPCache::Entry::kStatic);
}


unsigned arp_cache_dynamic_purge( struct sr_instance* sr ) {
  return arp_cache_type_purge(sr, ARPCache::Entry::kDynamic);
}


int router_interface_set_enabled( struct sr_instance* sr, const char* name, int enabled ) {
    fprintf( stderr, "not yet implemented: router_interface_set_enabled\n" );
    return -1;
}


void* router_lookup_interface_via_ip( struct sr_instance* sr,
                                      uint32_t ip ) {
    fprintf( stderr, "not yet implemented: router_lookup_interface_via_ip\n" );
    return NULL;
}


void* router_lookup_interface_via_name( struct sr_instance* sr,
                                        const char* name ) {
    fprintf( stderr, "not yet implemented: router_lookup_interface_via_name\n" );
    return NULL;
}


int router_is_interface_enabled( struct sr_instance* sr, void* intf ) {
    fprintf( stderr, "not yet implemented: router_is_interface_enabled\n" );
    return 0;
}


int router_is_ospf_enabled( struct sr_instance* sr ) {
    fprintf( stderr, "not yet implemented: router_is_ospf_enabled\n" );
    return 0;
}


void router_set_ospf_enabled( struct sr_instance* sr, int enabled ) {
    fprintf( stderr, "not yet implemented: router_set_ospf_enabled\n" );
}


void rtable_route_add( struct sr_instance* sr,
                       uint32_t dest, uint32_t gw, uint32_t mask,
                       void* intf,
                       int is_static_route ) {
    fprintf( stderr, "not yet implemented: rtable_route_add\n" );
}


int rtable_route_remove( struct sr_instance* sr,
                         uint32_t dest, uint32_t mask,
                         int is_static ) {
    fprintf( stderr, "not yet implemented: rtable_route_remove\n" );
    return 0 /* fail */;
}


void rtable_purge_all( struct sr_instance* sr ) {
    fprintf( stderr, "not yet implemented: rtable_purge_all\n" );
}


void rtable_purge( struct sr_instance* sr, int is_static ) {
    fprintf( stderr, "not yet implemented: rtable_purge\n" );
}
