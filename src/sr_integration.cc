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

#include <assert.h>

#include "fwk/buffer.h"
#include "fwk/log.h"

#include "data_plane.h"
#include "ethernet_packet.h"
#include "sr_vns.h"
#include "sr_base_internal.h"
#include "sw_data_plane.h"

#ifdef _CPUMODE_
#include "sr_cpu_extension_nf2.h"
#endif

static DataPlane::Ptr dp;
static Fwk::Log::Ptr log;


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
  log = Fwk::Log::LogNew("root");
  log->levelIs(log->debug());
  (*log)(log->info()) << "Initializing";

  /* TODO(ms): Differentiate based on _CPUMODE_. */
  dp = SWDataPlane::SWDataPlaneNew();
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
  (*log)(log->debug()) << "sw_integ_hw() called";
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
  (*log)(log->debug()) << "sr_integ_input() called";

  Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(packet, len);
  EthernetPacket::Ptr eth_pkt = EthernetPacket::EthernetPacketNew(buffer, 0);

  /* TODO(ms): bypass dataplane here on _CPUMODE_? */
  dp->packetNew(eth_pkt);
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
  (*log)(log->debug()) << "sr_integ_add_interface() called";


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
  (*log)(log->debug()) << "sr_integ_destroy() called";
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
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fprintf(stderr, "!!! Tranport layer called ip_findsrcip(..) this must be !!!\n");
    fprintf(stderr, "!!! defined to return the correct source address        !!!\n");
    fprintf(stderr, "!!! given a destination                                 !!!\n ");
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    assert(0);

    /* --
     * e.g.
     *
     * struct sr_instance* sr = sr_get_global_instance();
     * struct my_router* mr = (struct my_router*)
     *                              sr_get_subsystem(sr);
     * return my_findsrcip(mr, dest);
     * -- */

    return 0;
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
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fprintf(stderr, "!!! Tranport layer called sr_integ_ip_output(..)        !!!\n");
    fprintf(stderr, "!!! this must be defined to handle the network          !!!\n ");
    fprintf(stderr, "!!! level functionality of transport packets            !!!\n ");
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    assert(0);

    /* --
     * e.g.
     *
     * struct sr_instance* sr = sr_get_global_instance();
     * struct my_router* mr = (struct my_router*)
     *                              sr_get_subsystem(sr);
     * return my_ip_output(mr, payload, proto, src, dest, len);
     * -- */

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
  (*log)(log->debug()) << "sr_integ_close called";
}
