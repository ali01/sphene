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
#include <ctime>
#include <string>
#include <unistd.h>
#include <utility>

#include "fwk/buffer.h"
#include "fwk/concurrent_deque.h"
#include "fwk/exception.h"
#include "fwk/log.h"

#include "arp_cache.h"
#include "control_plane.h"
#include "data_plane.h"
#include "ethernet_packet.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"
#include "lwtcp/lwip/sys.h"
#include "routing_table.h"
#include "sr_vns.h"
#include "sr_base_internal.h"
#include "sw_data_plane.h"
#include "task.h"

#ifdef _CPUMODE_
#include "sr_cpu_extension_nf2.h"
#endif

using std::pair;
using std::string;

static ControlPlane::Ptr cp;
static DataPlane::Ptr dp;
static Fwk::Log::Ptr log_;
static Fwk::ConcurrentDeque<pair<EthernetPacket::Ptr,
                                 Interface::PtrConst> >::Ptr pq;
static TaskManager::Ptr tm;

static void processing_thread(void* aux);


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
  ILOG << "Initializing";

  // Create ControlPlane.
  cp = ControlPlane::ControlPlaneNew();

  // Create DataPlane.
  // TODO(ms): Differentiate based on _CPUMODE_.
  RoutingTable::Ptr routing_table = cp->routingTable();
  ARPCache::Ptr arp_cache = cp->arpCache();
  dp = SWDataPlane::SWDataPlaneNew(sr, routing_table, arp_cache);

  // Initialize pointers between ControlPlane and DataPlane.
  cp->dataPlaneIs(dp);
  dp->controlPlaneIs(cp.ptr());  // weak pointer to prevent circular reference

  // ControlPlane and DataPlane are parts of the router.
  sr->cp = cp.ptr();
  sr->dp = dp.ptr();

  // Initialize input packet queue.
  pq = Fwk::ConcurrentDeque<pair<EthernetPacket::Ptr,
                                 Interface::PtrConst> >::New();

  // Initialize task manager.
  tm = TaskManager::New();

  // Start processing thread.
  sys_thread_new(processing_thread, NULL);
}


/* Thread target for processing incoming packets and executing periodic
   tasks. Started by sr_integ_init(). */
static void processing_thread(void* aux) {
  DLOG << "processing thread started";
  struct timespec timeout_time;

  pair<EthernetPacket::Ptr, Interface::PtrConst> p;
  for (;;) {
    // Update timeout time.
    clock_gettime(CLOCK_REALTIME, &timeout_time);
    timeout_time.tv_sec += 1;

    try {
      p = pq->timedPopFront(timeout_time);

      EthernetPacket::Ptr eth_pkt = p.first;
      Interface::PtrConst iface = p.second;
      DLOG << "processing thread popped packet";

      // TODO(ms): bypass dataplane here on _CPUMODE_?
      dp->packetNew(eth_pkt, iface);
    } catch (Fwk::TimeoutException& e) {
      DLOG << "processing thread timeout";
    }
  }
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
  DLOG << "sw_integ_hw() called";
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
  DLOG << "sr_integ_input() called";

  // Find incoming interface.
  Interface::PtrConst iface = dp->interfaceMap()->interface(interface);
  if (!iface) {
    ELOG << "received packet on interface " << interface
         << ", but failed to find associated Interface object.";
    return;
  }

  Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(packet, len);
  EthernetPacket::Ptr eth_pkt = EthernetPacket::New(buffer, 0);

  // Insert packet into packet queue.
  pq->pushBack(std::make_pair(eth_pkt, iface));
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

  // Add the interface to the data plane.
  dp->interfaceMap()->interfaceIs(iface);

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
  DLOG << "sr_integ_destroy() called";
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
  RoutingTable::Ptr rtable = sr->cp->routingTable();

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
  Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(pkt_len);

  // Ethernet packet first. Src and Dst are set when the IP packet is sent.
  EthernetPacket::Ptr eth_pkt = EthernetPacket::New(buffer, 0);
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
  ip_pkt->ttlIs(64);

  // Copy in data.
  memcpy(ip_pkt->data() + IPPacket::kHeaderSize, payload, len);

  // Compute checksum.
  ip_pkt->checksumReset();

  // Send packet.
  sr->cp->outputPacketNew(ip_pkt);

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
