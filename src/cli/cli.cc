/* Filename: cli.c */

#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>               /* snprintf()                        */
#include <stdlib.h>              /* malloc()                          */
#include <string.h>              /* strncpy()                         */
#include <string>
#include <sys/time.h>            /* struct timeval                    */
#include <unistd.h>              /* sleep()                           */

#include "fwk/scoped_lock.h"

#include "cli.h"
#include "cli_network.h"         /* make_thread()                     */
#include "helper.h"
#include "socket_helper.h"       /* writenstr()                       */
#include "../sr_base_internal.h" /* struct sr_instance                */

#include "arp_cache.h"
#include "control_plane.h"
#include "interface.h"
#include "interface_map.h"
#include "nf2.h"
#include "nf2util.h"
#include "reg_defines.h"
#include "router.h"
#include "routing_table.h"
#include "sr_cpu_extension_nf2.h"
#include "tunnel.h"
#include "tunnel_map.h"

#include "cli_stubs.h"

using std::string;

/** whether to shutdown the server or not */
static int router_shutdown;

/** socket file descriptor where responses should be sent */
static int fd;

/** whether the fd is was terminated */
static int fd_alive;

/** whether the client is in verbose mode */
static int* pverbose;

/** whether to skip next prompt call */
static int skip_next_prompt;

static const char* const kDefaultIfaceName = "nf2c0";

#ifdef _STANDALONE_CLI_
/**
 * Initialize sr for the standalone binary which just runs the CLI.
 */
struct sr_instance* my_get_sr() {
    static struct sr_instance* sr = NULL;
    if( ! sr ) {
        sr = (struct sr_instance*)malloc( sizeof(*sr) );
        true_or_die( sr!=NULL, "malloc falied in my_get_sr" );

        fprintf( stderr, "not yet implemented: my_get_sr does not create a value for sr->interface_subsystem\n" );
        sr->interface_subsystem = NULL;

        sr->topo_id = 0;
        strncpy( sr->vhost, "cli", SR_NAMELEN );
        strncpy( sr->user, "cli mode (no client)", SR_NAMELEN );
        if( gethostname(sr->lhost,  SR_NAMELEN) == -1 )
            strncpy( sr->lhost, "cli mode (unknown localhost)", SR_NAMELEN );

        /* NOTE: you probably want to set some dummy values for the rtable and
           interface list of your interface_subsystem here (preferably read them
           from a file) */
    }

    return sr;
}
#   define SR my_get_sr()
#else
#   include "../sr_integration.h" /* sr_get() */
#   define SR get_sr()
#endif

/**
 * Wrapper for writenstr.  Tries to send the specified string with the
 * file-scope fd.  If it fails, fd_alive is set to 0.  Does nothing if
 * fd_alive is already 0.
 */
static void cli_send_str( const char* str ) {
    if( fd_alive )
        if( 0 != writenstr( fd, str ) )
            fd_alive = 0;
}

#ifdef _VNS_MODE_
/**
 * Wrapper for writenstr.  Tries to send the specified string followed by a
 * newline with the file-scope fd.  If it fails, fd_alive is set to 0.  Does
 * nothing if fd_alive is already 0.
 */
static void cli_send_strln( const char* str ) {
    if( fd_alive )
        if( 0 != writenstrs( fd, 2, str, "\n" ) )
            fd_alive = 0;
}
#endif

/**
 * Wrapper for writenstrs.  Tries to send the specified string(s) with the
 * file-scope fd.  If it fails, fd_alive is set to 0.  Does nothing if
 * fd_alive is already 0.
 */
static void cli_send_strs( int num_args, ... ) {
    const char* str;
    int ret;
    va_list args;

    if( !fd_alive ) return;
    va_start( args, num_args );

    ret = 0;
    while( ret==0 && num_args-- > 0 ) {
        str = va_arg(args, const char*);
        ret = writenstr( fd, str );
    }

    va_end( args );
    if( ret != 0 )
        fd_alive = 0;
}

void cli_init() {
    router_shutdown = 0;
    skip_next_prompt = 0;
}

int cli_is_time_to_shutdown() {
    return router_shutdown;
}

int cli_focus_is_alive() {
    return fd_alive;
}

void cli_focus_set( const int sfd, int* verbose ) {
    fd_alive = 1;
    fd = sfd;
    pverbose = verbose;
}

void cli_send_help( cli_help_t help_type ) {
    if( fd_alive )
        if( !cli_send_help_to( fd, help_type ) )
            fd_alive = 0;
}

void cli_send_parse_error( int num_args, ... ) {
    const char* str;
    int ret;
    va_list args;

    if( fd_alive ) {
        va_start( args, num_args );

        ret = 0;
        while( ret==0 && num_args-- > 0 ) {
            str = va_arg(args, const char*);
            ret = writenstr( fd, str );
        }

        va_end( args );
        if( ret != 0 )
            fd_alive = 0;
    }
}

void cli_send_welcome() {
    cli_send_str( "You are now logged into the router CLI.\n" );
}

void cli_send_prompt() {
    if( !skip_next_prompt )
        cli_send_str( PROMPT );

    skip_next_prompt = 0;
}

void cli_show_all() {
#ifdef _CPUMODE_
    cli_show_hw();
    cli_send_str( "\n" );
#endif
    cli_show_ip();
#ifndef _CPUMODE_
#ifndef _MANUAL_MODE_
    cli_send_str( "\n" );
    cli_show_vns();
#endif
#endif
}

#ifndef _CPUMODE_
void cli_send_no_hw_str() {
    cli_send_str( "HW information is not available when not in CPU mode\n" );
}
#else
void cli_show_hw() {
    cli_send_str( "HW State:\n" );
    cli_show_hw_about();
    cli_show_hw_arp();
    cli_show_hw_intf();
    cli_show_hw_route();
}

void cli_show_hw_about() {
    cli_send_str( "not yet implemented: cli_show_hw_about()\n" );
}

void cli_show_hw_arp() {
  char line_buf[256];
  const char* const format = "  %-2d  %-16s   %-19s\n";

  // Open the NetFPGA for reading registers.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  cli_send_str("HW ARP cache:\n");
  for (unsigned int index = 0; index < ARPCache::kMaxEntries; ++index) {
    // Set index.
    writeReg(&nf2, ROUTER_OP_LUT_ARP_TABLE_WR_ADDR, index);

    unsigned int mac_hi;
    unsigned int mac_lo;
    readReg(&nf2, ROUTER_OP_LUT_ARP_TABLE_ENTRY_MAC_HI, &mac_hi);
    readReg(&nf2, ROUTER_OP_LUT_ARP_TABLE_ENTRY_MAC_LO, &mac_lo);

    // Read MAC address.
    uint8_t mac_addr[6];
    mac_addr[0] = (mac_hi & 0x0000FF00) >> 8;
    mac_addr[1] = (mac_hi & 0x000000FF);
    mac_addr[2] = (mac_lo & 0xFF000000) >> 24;
    mac_addr[3] = (mac_lo & 0x00FF0000) >> 16;
    mac_addr[4] = (mac_lo & 0x0000FF00) >> 8;
    mac_addr[5] = (mac_lo & 0x000000FF);

    // Read IP address.
    uint32_t ip_addr;
    readReg(&nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_NEXT_HOP_IP, &ip_addr);

    EthernetAddr mac(mac_addr);
    IPv4Addr ip(ip_addr);

    if (mac != "00:00:00:00:00:00" && ip != 0) {
      snprintf(line_buf, sizeof(line_buf), format,
               index, string(ip).c_str(), string(mac).c_str());
      cli_send_str(line_buf);
    }
  }

  closeDescriptor(&nf2);
}

void cli_show_hw_intf() {
  char line_buf[256];
  const char* const format = "  %-2d  %-16s\n";

  // Open the NetFPGA for reading registers.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  cli_send_str("HW IP filter table:\n");
  for (unsigned int index = 0; index < InterfaceMap::kMaxEntries; ++index) {
    // Set index.
    writeReg(&nf2, ROUTER_OP_LUT_DST_IP_FILTER_TABLE_WR_ADDR, index);

    // Read IP address.
    uint32_t ip_addr;
    readReg(&nf2, ROUTER_OP_LUT_DST_IP_FILTER_TABLE_ENTRY_IP, &ip_addr);

    IPv4Addr ip(ip_addr);

    if (ip != 0) {
      snprintf(line_buf, sizeof(line_buf), format,
               index, string(ip).c_str());
      cli_send_str(line_buf);
    }
  }

  closeDescriptor(&nf2);
}

void cli_show_hw_route() {
    cli_send_str( "not yet implemented: cli_show_hw_route()\n" );
}
#endif

void cli_show_ip() {
    cli_send_str("IP State:\n");
    cli_show_ip_arp();
    cli_send_str("\n");
    cli_show_ip_intf();
    cli_send_str("\n");
    cli_show_ip_route();
    cli_send_str("\n");
    cli_show_ip_tunnel();
}

void cli_show_ip_arp() {
  struct sr_instance* sr = get_sr();
  ARPCache::Ptr cache = sr->router->controlPlane()->arpCache();

  // Buffers for proper formatting.
  char line_buf[256];
  std::stringstream ss;

  // Line format.
  const char* const format = "  %-16s   %-19s\n";

  // Output header.
  cli_send_str("ARP cache:\n");
  snprintf(line_buf, sizeof(line_buf), format,
           "IP address", "MAC address");
  cli_send_str(line_buf);

  {
    Fwk::ScopedLock<ARPCache> lock(cache);

    for (ARPCache::iterator it = cache->begin(); it != cache->end(); ++it) {
      ARPCache::Entry::Ptr entry = it->second;
      const string& ip = entry->ipAddr();
      const string& mac = entry->ethernetAddr();

      snprintf(line_buf, sizeof(line_buf), format,
               ip.c_str(), mac.c_str());
      ss << line_buf;
    }
  }

  // Send to client.
  cli_send_str(ss.str().c_str());
}

void cli_show_ip_intf() {
  struct sr_instance* sr = get_sr();
  InterfaceMap::Ptr ifaces = sr->router->dataPlane()->interfaceMap();

  // Buffer for proper formatting.
  char line_buf[256];
  std::stringstream ss;

  // Line format.
  const char* const format = "  %-6s %-16s %-16s %-19s %-6s\n";

  // Output header.
  cli_send_str("Interfaces:\n");
  snprintf(line_buf, sizeof(line_buf), format,
           "Name", "IP address", "Mask", "MAC address", "Status");
  cli_send_str(line_buf);

  {
    Fwk::ScopedLock<InterfaceMap> lock(ifaces);

    for (InterfaceMap::const_iterator it = ifaces->begin();
         it != ifaces->end(); ++it) {
      Interface::Ptr iface = it->second;
      const string& name = iface->name();
      const string& ip = iface->ip();
      const string& mask = iface->subnetMask();
      const string& mac = iface->mac();
      const bool enabled = iface->enabled();

      snprintf(line_buf, sizeof(line_buf), format,
               name.c_str(), ip.c_str(), mask.c_str(), mac.c_str(),
               (enabled ? "up" : "down"));
      ss << line_buf;
    }
  }

  cli_send_str(ss.str().c_str());
}

void cli_show_ip_route() {
  struct sr_instance* sr = get_sr();
  RoutingTable::Ptr rtable = sr->router->controlPlane()->routingTable();

  // Buffers for proper formatting.
  char line_buf[256];
  std::stringstream ss;

  // Line format.
  const char* const format = "  %-16s %-16s %-16s %-11s %-9s\n";

  // Output header.
  cli_send_str("Routing table:\n");
  snprintf(line_buf, sizeof(line_buf), format,
           "Destination", "Gateway", "Mask", "Interface", "Type");
  cli_send_str(line_buf);

  // Build up stringstream of routing table entries. We cannot output to the
  // client here because routing the outgoing packet would require the the
  // routing table lock, which we hold.
  {
    Fwk::ScopedLock<RoutingTable> lock(rtable);

    RoutingTable::const_iterator it;
    for (it = rtable->entriesBegin(); it != rtable->entriesEnd(); ++it) {
      RoutingTable::Entry::Ptr entry = it->second;
      const string& subnet = entry->subnet();
      const string& mask = entry->subnetMask();
      const string& gateway = entry->gateway();
      const string& if_name = entry->interface()->name();
      const RoutingTable::Entry::Type type = entry->type();

      snprintf(line_buf, sizeof(line_buf), format,
               subnet.c_str(), gateway.c_str(), mask.c_str(), if_name.c_str(),
               ((type == RoutingTable::Entry::kStatic) ? "static" : "dynamic"));
      ss << line_buf;
    }
  }

  // Output to client.
  cli_send_str(ss.str().c_str());
}

void cli_show_ip_tunnel() {
  struct sr_instance* sr = get_sr();
  TunnelMap::Ptr tun_map = sr->router->controlPlane()->tunnelMap();

  // Buffer for proper formatting.
  char line_buf[256];
  std::stringstream ss;

  // Line format.
  const char* const format = "  %-16s %-9s\n";

  // Output header.
  cli_send_str("Tunnels:\n");
  snprintf(line_buf, sizeof(line_buf), format, "Endpoint IP", "Interface");
  cli_send_str(line_buf);

  {
    Fwk::ScopedLock<TunnelMap> lock(tun_map);

    for (TunnelMap::const_iterator it = tun_map->begin();
         it != tun_map->end(); ++it) {
      Tunnel::Ptr tunnel = it->second;
      const string& remote = tunnel->remote();
      const string& if_name = tunnel->interface()->name();

      snprintf(line_buf, sizeof(line_buf), format,
               remote.c_str(), if_name.c_str());
      ss << line_buf;
    }
  }

  cli_send_str(ss.str().c_str());
}

void cli_show_opt() {
    cli_show_opt_verbose();
}

void cli_show_opt_verbose() {
    if( *pverbose )
        cli_send_str( "Verbose: Enabled\n" );
    else
        cli_send_str( "Verbose: Disabled\n" );
}

void cli_show_ospf() {
    cli_send_str( "Neighbor Information:\n" );
    cli_show_ospf_neighbors();

    cli_send_str( "Topology:\n" );
    cli_show_ospf_topo();
}

void cli_show_ospf_neighbors() {
    cli_send_str( "not yet implemented: show list of neighbors for each interface of SR\n" );
}

void cli_show_ospf_topo() {
    cli_send_str( "not yet implemented: show PWOSPF topology of SR (e.g., for each router, show its ID, last pwospf seq #, and a list of all its links (e.g., router ID + subnet))\n" );
}

#ifndef _VNS_MODE_
void cli_send_no_vns_str() {
#ifdef _CPUMODE_
    cli_send_str( "VNS information is not available when in CPU mode\n" );
#else
    cli_send_str( "VNS information is not available when in Manual mode\n" );
#endif
}
#else
void cli_show_vns() {
    cli_send_str( "VNS State:\n  Localhost: " );
    cli_show_vns_lhost();
    cli_send_str( "  Topology: " );
    cli_show_vns_topo();
    cli_send_str( "  User: " );
    cli_show_vns_user();
    cli_send_str( "  Virtual Host: " );
    cli_show_vns_vhost();
}

void cli_show_vns_lhost() {
    cli_send_strln( SR->lhost );
}

void cli_show_vns_topo() {
    char buf[7];
    snprintf( buf, 7, "%u\n", SR->topo_id );
    cli_send_str( buf );
}

void cli_show_vns_user() {
    cli_send_strln( SR->user );
}

void cli_show_vns_vhost() {
    cli_send_strln( SR->vhost );
}
#endif

void cli_manip_ip_arp_add( gross_arp_t* data ) {
    char ip[STRLEN_IP];
    char mac[STRLEN_MAC];

    ip_to_string( ip, data->ip );
    mac_to_string( mac, data->mac );

    if( arp_cache_static_entry_add( SR, data->ip, data->mac ) )
        cli_send_strs( 5, "Added translation of ", ip, " <-> ", mac, " to the static ARP cache\n" );
    else
        cli_send_strs( 5, "Error: Unable to add a translation of ", ip, " <-> ", mac,
                       " to the static ARP cache -- try removing another static entry first.\n" );
}

void cli_manip_ip_arp_del( gross_arp_t* data ) {
    char ip[STRLEN_IP];

    ip_to_string( ip, data->ip );
    if( arp_cache_static_entry_remove( SR, data->ip ) )
        cli_send_strs( 3, "Removed ", ip, " from the ARP cache\n" );
    else
        cli_send_strs( 3, "Error: ", ip, " was not a static ARP cache entry\n" );
}

void cli_manip_ip_arp_purge_all() {
    int countD, countS, countT;
    char str_countS[11];
    char str_countT[11];
    const char* whatS;
    const char* whatT;

    countD = arp_cache_dynamic_purge( SR );

    countS = arp_cache_static_purge( SR );
    whatS = ( countS == 1 ) ? " entry" : " entries";
    snprintf( str_countS, 11, "%u", countS );

    countT = countD + countS;
    whatT = ( countT == 1 ) ? " entry" : " entries";
    snprintf( str_countT, 11, "%u", countT );

    cli_send_strs( 8, "Removed ", str_countT, whatT,
                   " (", str_countS, " static", whatS, ") from the ARP cache\n" );
}

void cli_manip_ip_arp_purge_dyn() {
    int count;
    char str_count[11];
    const char* what;

    count = arp_cache_dynamic_purge( SR );
    what = ( count == 1 ) ? " entry" : " entries";
    snprintf( str_count, 11, "%u", count );
    cli_send_strs( 4, "Removed ", str_count, what, " from the ARP cache\n" );
}

void cli_manip_ip_arp_purge_sta() {
    int count;
    char str_count[11];
    const char* what;

    count = arp_cache_static_purge( SR );
    what = ( count == 1 ) ? " entry" : " entries";
    snprintf( str_count, 11, "%u", count );
    cli_send_strs( 5, "Removed ", str_count, " static", what, " from the ARP cache\n" );
}

// TODO(ms): This should probably be implemented in cli_stubs.{cc,h}.
void cli_manip_ip_intf_set( gross_intf_t* data ) {
    Interface::Ptr intf =
        router_lookup_interface_via_name( SR, data->intf_name );
    if (intf) {
      InterfaceMap::Ptr ifaces = SR->router->dataPlane()->interfaceMap();
      Fwk::ScopedLock<InterfaceMap> lock(ifaces);

      // Remove the interface from the map before we update it.
      ifaces->interfaceDel(intf);

      // Update interface attributes.
      intf->ipIs(ntohl(data->ip));
      intf->subnetMaskIs(ntohl(data->subnet_mask));

      // Re-add the interface to the map.
      ifaces->interfaceIs(intf);

      // TODO(ms): This.
      /* not yet implemented: let everyone else know the routes we offer have changed */
    } else {
      cli_send_strs( 2, data->intf_name, " is not a valid interface\n" );
    }
}

void cli_manip_ip_intf_set_enabled( const char* intf_name, int enabled ) {
    int ret;
    const char* what;

    ret = router_interface_set_enabled( SR, intf_name, enabled );
    what = (enabled ? "enabled\n" : "disabled\n");

    switch( ret ) {
    case 0:
        cli_send_strs( 3, intf_name, " has been ", what );
        break;

    case 1:
        cli_send_strs( 3, intf_name, " was already ", what );

    case -1:
    default:
        cli_send_strs( 2, intf_name, " is not a valid interface\n" );
    }
}

void cli_manip_ip_intf_down( gross_intf_t* data ) {
    cli_manip_ip_intf_set_enabled( data->intf_name, 0 );
}

void cli_manip_ip_intf_up( gross_intf_t* data ) {
    cli_manip_ip_intf_set_enabled( data->intf_name, 1 );
}

void cli_manip_ip_ospf_down() {
    if( router_is_ospf_enabled( SR ) ) {
        router_set_ospf_enabled( SR, 0 );
        cli_send_str( "OSPF has been disabled" );
    }
    else
        cli_send_str( "OSPF was already disabled" );
}

void cli_manip_ip_ospf_up() {
    if( !router_is_ospf_enabled( SR ) ) {
        router_set_ospf_enabled( SR, 1 );
        cli_send_str( "OSPF has been enabled" );
    }
    else
        cli_send_str( "OSPF was already enabled" );
}

void cli_manip_ip_route_add( gross_route_t* data ) {
    Interface::Ptr intf =
        router_lookup_interface_via_name( SR, data->intf_name );
    if( !intf )
        cli_send_strs( 3, "Error: no interface with the name ",
                       data->intf_name, " exists.\n" );
    else {
        rtable_route_add(SR, data->dest, data->gw, data->mask, intf, 1);
        cli_send_str( "The route has been added.\n" );
    }
}

void cli_manip_ip_route_del( gross_route_t* data ) {
    if( rtable_route_remove( SR, data->dest, data->mask, 1 ) )
        cli_send_str( "The route has been removed.\n" );
    else
        cli_send_str( "That route does not exist.\n" );
}

void cli_manip_ip_route_purge_all() {
    rtable_purge_all( SR );
    cli_send_str( "All routes have been removed from the routing table.\n" );
}

void cli_manip_ip_route_purge_dyn() {
    rtable_purge( SR, 0 );
    cli_send_str( "All dymanic routes have been removed from the routing table.\n" );
}

void cli_manip_ip_route_purge_sta() {
    rtable_purge( SR, 1 );
    cli_send_str( "All static routes have been removed from the routing table.\n" );
}

void cli_manip_ip_tunnel_add(gross_tunnel_t* data) {
  if (strcmp(data->mode, "gre")) {
    cli_send_str("Unknown mode: ");
    cli_send_str(data->mode);
    cli_send_str("\n");
    return;
  }

  if (tunnel_add(SR, data->name, data->mode, data->remote))
    cli_send_str("The tunnel has been added\n");
  else
    cli_send_str("An interface or tunnel already exists with that name.\n");
}

void cli_manip_ip_tunnel_del(gross_tunnel_t* data) {
  if (tunnel_del(SR, data->name))
    cli_send_str("The tunnel has been removed.\n");
  else
    cli_send_str("That tunnel does not exist.\n");
}

void cli_manip_ip_tunnel_change(gross_tunnel_t* data) {
  if (strcmp(data->mode, "gre")) {
    cli_send_str("Unknown mode: ");
    cli_send_str(data->mode);
    cli_send_str("\n");
    return;
  }

  if (tunnel_change(SR, data->name, data->mode, data->remote))
    cli_send_str("The tunnel has been changed.\n");
  else
    cli_send_str("No tunnel exists with that name.\n");
}

void cli_date() {
    char str_time[STRLEN_TIME];
    struct timeval now;

    gettimeofday( &now, NULL );
    time_to_string( str_time, now.tv_sec );
    cli_send_str( str_time );
}

void cli_exit() {
    cli_send_str( "Goodbye!\n" );
    fd_alive = 0;
}

int cli_ping_handle_self( uint32_t ip ) {
    Interface::Ptr intf = router_lookup_interface_via_ip( SR, ip );
    if( intf ) {
        if( router_is_interface_enabled( SR, intf ) )
            cli_send_str( "Your interface is up.\n" );
        else
            cli_send_str( "Your interface is down.\n" );

        return 1;
    }

    return 0;
}

/**
 * Sends a ping to the specified IP address.  Information about it being sent
 * and whether it succeeds or not should be sent to the specified client_fd.
 */
static void cli_send_ping( int client_fd, uint32_t ip ) {
    fprintf( stderr, "not yet implmented: send an echo request\n" );
}

void cli_ping( gross_ip_t* data ) {
    if( cli_ping_handle_self( data->ip ) )
        return;

    cli_send_ping(fd, data->ip );
    skip_next_prompt = 1;
}

void cli_ping_flood( gross_ip_int_t* data ) {
    unsigned int i;
    char str_ip[STRLEN_IP];

    if( cli_ping_handle_self( data->ip ) )
        return;

    ip_to_string( str_ip, data->ip );
    if( 0 != writenf( fd, "Will ping %s %u times ...\n", str_ip, data->count ) )
        fd_alive = 0;

    for( i=0; i<data->count; i++ )
        cli_send_ping( fd, data->ip );
    skip_next_prompt = 1;
}

void cli_shutdown() {
    cli_send_str( "Shutting down the router ...\n" );
    router_shutdown = 1;

    /* we could do a cleaner shutdown, but this is probably fine */
    exit(0);
}

void cli_traceroute( gross_ip_t* data ) {
    cli_send_str( "not yet implemented: traceroute\n" );
}

void cli_opt_verbose( gross_option_t* data ) {
    if( data->on ) {
        if( *pverbose )
            cli_send_str( "Verbose mode is already enabled.\n" );
        else {
            *pverbose = 1;
            cli_send_str( "Verbose mode is now enabled.\n" );
        }
    }
    else {
        if( *pverbose ) {
            *pverbose = 0;
            cli_send_str( "Verbose mode is now disabled.\n" );
        }
        else
            cli_send_str( "Verbose mode is already disabled.\n" );
    }
}
