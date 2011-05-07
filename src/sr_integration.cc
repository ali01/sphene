/*-----------------------------------------------------------------------------
 * file:  sr_integration.c
 * date:  Tue Feb 03 11:29:17 PST 2004
 * Author: Martin Casado <casado@stanford.edu>
 *
 * Description:
 *
 * Methods called by the lowest-level of the network system to talk with
 * the network subsystem.
 *
 * This is the entry point of integration for the network layer.
 *
 *---------------------------------------------------------------------------*/

#include <stdlib.h>

#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>

#include "fwk/concurrent_deque.h"
#include "fwk/exception.h"
#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "arp_cache.h"
#include "arp_cache_daemon.h"
#include "arp_queue_daemon.h"
#include "control_plane.h"
#include "data_plane.h"
#include "ethernet_packet.h"
#include "hw_data_plane.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"
#include "lwtcp/lwip/sys.h"
#include "ospf_daemon.h"
#include "ospf_router.h"
#include "packet_buffer.h"
#include "router.h"
#include "routing_table.h"
#include "sr_cpu_extension_nf2.h"
#include "sr_vns.h"
#include "sr_base_internal.h"
#include "sw_data_plane.h"
#include "task.h"

using std::pair;
using std::string;

static Fwk::Log::Ptr log_;

static void processing_thread(void* _sr);
static void read_rtable(struct sr_instance* sr);


/*-----------------------------------------------------------------------------
 * Method: sr_integ_init(..)
 * Scope: global
 *
 *
 * First method called during router initialization.  Called before connecting
 * to VNS, reading in hardware information etc.
 *
 *---------------------------------------------------------------------------*/

void sr_integ_init(struct sr_instance* sr)
{
  log_ = Fwk::Log::LogNew("root");
  log_->levelIs(log_->debug());

  // Create ControlPlane.
  ControlPlane::Ptr cp = ControlPlane::ControlPlaneNew();

  // Create DataPlane.
#ifdef _CPUMODE_
  DataPlane::Ptr dp = HWDataPlane::New(sr, cp->routingTable(), cp->arpCache());
#else
  DataPlane::Ptr dp =
      SWDataPlane::SWDataPlaneNew(sr, cp->routingTable(), cp->arpCache());
#endif

  // Initialize task manager.
  TaskManager::Ptr tm = TaskManager::New();

  // Create Router in the given sr_instance.
  sr->router = Router::New("Router", cp, dp, tm);

  // Initialize input packet queue.
  sr->input_queue = sr_instance::PacketQueue::New();
}


/* Thread target for processing incoming packets and executing periodic
   tasks. Started by sr_integ_init(). */
static void processing_thread(void* _sr) {
  struct sr_instance* sr = (struct sr_instance*)_sr;
  Router::Ptr router = sr->router;

  sr->processing_thread_running = true;
  DLOG << "Processing thread started";
  struct timespec last_time;
  struct timespec next_time;
  clock_gettime(CLOCK_REALTIME, &last_time);

  pair<EthernetPacket::Ptr, Interface::PtrConst> p;
  while (!sr->quit) {
    // Get current time.
    clock_gettime(CLOCK_REALTIME, &next_time);

    // Ensure we run tasks in the task manager every second.
    if (next_time.tv_sec - last_time.tv_sec >= 1) {
      router->taskManager()->timeIs(time(NULL));
      last_time = next_time;
    }

    try {
      // Bound the waiting time for a packet in the input queue.
      next_time.tv_sec += 1;
      p = sr->input_queue->timedPopFront(next_time);

      EthernetPacket::Ptr eth_pkt = p.first;
      Interface::PtrConst iface = p.second;
      DLOG << "Processing thread popped packet";

      // TODO(ms): bypass dataplane here on _CPUMODE_?
      router->dataPlane()->packetNew(eth_pkt, iface);
    } catch (Fwk::TimeoutException& e) {
      // Timeout while waiting for a packet in the input queue. Ignore it.
    }
  }

  sr->processing_thread_running = false;
  DLOG << "Processing thread exiting";
}


/*-----------------------------------------------------------------------------
 * Method: sr_integ_hw_setup(..)
 * Scope: global
 *
 * Called after all initial hardware information (interfaces) have been
 * received.  Can be used to start subprocesses (such as dynamic-routing
 * protocol) which require interface information during initialization.
 *
 *---------------------------------------------------------------------------*/

void sr_integ_hw_setup(struct sr_instance* sr)
{
  Router::Ptr router = sr->router;

  // Read in rtable file, if any.
  read_rtable(sr);

  // Create ARP cache daemon and add it to the task manager.
  ARPCacheDaemon::Ptr arp_cache_daemon =
      ARPCacheDaemon::New(router->controlPlane()->arpCache());
  arp_cache_daemon->periodIs(1);  // check for stale entries every second
  router->taskManager()->taskIs(arp_cache_daemon);

  // Create ARP queue daemon and add it to the task manager.
  ARPQueueDaemon::Ptr arp_queue_daemon =
      ARPQueueDaemon::New(router->controlPlane());
  arp_queue_daemon->periodIs(1);
  router->taskManager()->taskIs(arp_queue_daemon);

  // Create OSPF daemon and add it to the task manager.
  // TODO(ms): Use real OSPFRouter instance here.
  OSPFDaemon::Ptr ospf_daemon =
    OSPFDaemon::New(NULL, router->controlPlane(), router->dataPlane());
  ospf_daemon->periodIs(1);
  // TODO(ali): enable OSPFDaemon
  //router->taskManager()->taskIs(ospf_daemon);

  // Start processing thread.
  sr->quit = false;
  sys_thread_new(processing_thread, sr);
}


static void read_rtable(struct sr_instance* const sr) {
  RoutingTable::Ptr rt = sr->router->controlPlane()->routingTable();
  char subnet[16];
  char gateway[16];
  char mask[16];
  char iface_name[16];

  std::ifstream ifs(sr->rtable, std::ifstream::in);
  if (ifs.good())
    DLOG << "Reading rtable file: " << sr->rtable;

  while (ifs.good()) {
    ifs >> subnet >> gateway >> mask >> iface_name;
    if (!ifs.good())
      break;  // avoid entering the last route twice

    // Lookup interface by name.
    Interface::Ptr iface =
        sr->router->dataPlane()->interfaceMap()->interface(iface_name);
    if (!iface) {
      WLOG << "No interface found by name " << iface_name << "; skipping";
      continue;
    }

    // Add new routing entry.
    RoutingTable::Entry::Ptr entry =
      RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
    entry->subnetIs(subnet, mask);
    entry->gatewayIs(gateway);
    entry->interfaceIs(iface);

    {
      Fwk::ScopedLock<RoutingTable>lock(rt);
      rt->entryIs(entry);
    }

    DLOG << "Added route: " << entry->subnet() << "/" << entry->subnetMask()
         << " gw " << entry->gateway();
  }

  ifs.close();
}


/*---------------------------------------------------------------------
 * Method: sr_integ_input(struct sr_instance*,
 *                        uint8_t* packet,
 *                        char* interface)
 * Scope:  Global
 *
 * This method is called each time the router receives a packet on the
 * interface.  The packet buffer, the packet length and the receiving
 * interface are passed in as parameters. The packet is complete with
 * ethernet headers.
 *
 * Note: Both the packet buffer and the character's memory are handled
 * by sr_vns_comm.c that means do NOT delete either.  Make a copy of the
 * packet instead if you intend to keep it around beyond the scope of
 * the method call.
 *
 *---------------------------------------------------------------------*/

void sr_integ_input(struct sr_instance* sr,
                    const uint8_t * packet/* borrowed */,
                    unsigned int len,
                    const char* interface/* borrowed */)
{
  DLOG << "Received packet";

  // Find incoming interface.
  Interface::PtrConst iface =
      sr->router->dataPlane()->interfaceMap()->interface(interface);
  if (!iface) {
    ELOG << "received packet on interface " << interface
         << ", but failed to find associated Interface object.";
    return;
  }

  PacketBuffer::Ptr buffer = PacketBuffer::New(packet, len);
  EthernetPacket::Ptr eth_pkt =
      EthernetPacket::New(buffer, buffer->size() - len);

  // Insert packet into packet queue.
  sr->input_queue->pushBack(std::make_pair(eth_pkt, iface));
}

/*-----------------------------------------------------------------------------
 * Method: sr_integ_add_interface(..)
 * Scope: global
 *
 * Called for each interface read in during hardware initialization.
 * struct sr_vns_if is defined in sr_base_internal.h
 *
 *---------------------------------------------------------------------------*/

void sr_integ_add_interface(struct sr_instance* sr,
                            struct sr_vns_if* vns_if/* borrowed */)
{
  // Create an Interface from vns_if data.
  Interface::Ptr iface = Interface::InterfaceNew(vns_if->name);
  iface->macIs(vns_if->addr);
  iface->ipIs(ntohl(vns_if->ip));  // vns_if->ip and friends are nbo
  iface->subnetMaskIs(ntohl(vns_if->mask));
  iface->speedIs(vns_if->speed);
  iface->typeIs(Interface::kHardware);

  // Add the interface to the data plane.
  sr->router->dataPlane()->interfaceMap()->interfaceIs(iface);

  DLOG << "Added interface " << iface->name();
  DLOG << "  mac: " << iface->mac();
  DLOG << "  ip: " << (string)iface->ip();
  DLOG << "  mask: " << (string)iface->subnetMask();
  DLOG << "  speed: " << iface->speed();
}

struct sr_instance* get_sr() {
    struct sr_instance* sr;

    sr = sr_get_global_instance( NULL );
    assert( sr );
    return sr;
}

/*-----------------------------------------------------------------------------
 * Method: sr_integ_low_level_output(..)
 * Scope: global
 *
 * Send a packet to VNS to be injected into the topology
 *
 *---------------------------------------------------------------------------*/

int sr_integ_low_level_output(struct sr_instance* sr /* borrowed */,
                             uint8_t* buf /* borrowed */ ,
                             unsigned int len,
                             const char* iface /* borrowed */)
{
#ifdef _CPUMODE_
    return sr_cpu_output(sr, buf /*lent*/, len, iface);
#else
    return sr_vns_send_packet(sr, buf /*lent*/, len, iface);
#endif /* _CPUMODE_ */
}

/*-----------------------------------------------------------------------------
 * Method: sr_integ_destroy(..)
 * Scope: global
 *
 * For memory deallocation pruposes on shutdown.
 *
 *---------------------------------------------------------------------------*/

void sr_integ_destroy(struct sr_instance* sr)
{
  // Kill processing thread.
  sr->quit = true;
  while (sr->processing_thread_running)
    sleep(1);

  // Destroy queue.
  sr->input_queue->clear();
  sr->input_queue = NULL;

  // Destroy router.
  sr->router = NULL;
}

/*-----------------------------------------------------------------------------
 * Method: sr_integ_findsrcip(..)
 * Scope: global
 *
 * Called by the transport layer for outgoing packets generated by the
 * router.  Expects source address in network byte order.
 *
 *---------------------------------------------------------------------------*/

uint32_t sr_integ_findsrcip(uint32_t dest /* nbo */)
{
  struct sr_instance* sr = sr_get_global_instance(NULL);
  RoutingTable::Ptr rtable = sr->router->controlPlane()->routingTable();
  Fwk::ScopedLock<RoutingTable> lock(rtable);

  // Find routing table entry for destination.
  IPv4Addr dest_ip(ntohl(dest));
  RoutingTable::Entry::Ptr entry = rtable->lpm(dest_ip);

  if (!entry) {
    // No route for destination.
    return 0;
  }

  // Source address is the IP address of the entry's interface.
  IPv4Addr src_ip = entry->interface()->ip();

  return src_ip.nbo();
}


/*-----------------------------------------------------------------------------
 * Method: sr_integ_ip_output(..)
 * Scope: global
 *
 * Called by the transport layer for outgoing packets that need IP
 * encapsulation.
 *
 *---------------------------------------------------------------------------*/

uint32_t sr_integ_ip_output(uint8_t* payload /* given */,
                            uint8_t  proto,
                            uint32_t src, /* nbo */
                            uint32_t dest, /* nbo */
                            int len)
{
  struct sr_instance* sr = sr_get_global_instance(NULL);

    // Create buffer for new packet.
  const size_t pkt_len = (EthernetPacket::kHeaderSize +
                          IPPacket::kHeaderSize +
                          len);
  PacketBuffer::Ptr buffer = PacketBuffer::New(pkt_len);

  // Ethernet packet first. Src and Dst are set when the IP packet is sent.
  EthernetPacket::Ptr eth_pkt =
      EthernetPacket::New(buffer, buffer->size() - pkt_len);
  eth_pkt->typeIs(EthernetPacket::kIP);

  // IP packet next.
  IPPacket::Ptr ip_pkt =
      IPPacket::Ptr::st_cast<IPPacket>(eth_pkt->payload());
  ip_pkt->versionIs(4);
  ip_pkt->headerLengthIs(IPPacket::kHeaderSize / 4);  // words, not bytes!
  ip_pkt->packetLengthIs(IPPacket::kHeaderSize + len);
  ip_pkt->diffServicesAre(0);
  ip_pkt->protocolIs((IPPacket::IPType)proto);
  ip_pkt->flagsAre(0);
  ip_pkt->fragmentOffsetIs(0);
  ip_pkt->srcIs(ntohl(src));
  ip_pkt->dstIs(ntohl(dest));
  ip_pkt->ttlIs(IPPacket::kDefaultTTL);

  // Copy in data.
  memcpy(ip_pkt->data() + IPPacket::kHeaderSize, payload, len);

  // Compute checksum.
  ip_pkt->checksumReset();

  // Send packet.
  sr->router->controlPlane()->outputPacketNew(ip_pkt);

  // The payload pointer was given to us.
  free(payload);

  return 0;
}

/*-----------------------------------------------------------------------------
 * Method: sr_integ_close(..)
 * Scope: global
 *
 *  Called when the router is closing connection to VNS.
 *
 *---------------------------------------------------------------------------*/

void sr_integ_close(struct sr_instance* sr)
{
  DLOG << "sr_integ_close called";
}
