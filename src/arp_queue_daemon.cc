#include "arp_queue_daemon.h"

#include <vector>

#include "arp_queue.h"
#include "control_plane.h"
#include "ethernet_packet.h"
#include "fwk/log.h"
#include "ip_packet.h"
#include "task.h"
#include "time_types.h"

static const unsigned int kMaxRetries = 4;

using std::vector;


ARPQueueDaemon::ARPQueueDaemon(ControlPlane::Ptr cp)
    : PeriodicTask("ARPQueueDaemon"),
      cp_(cp),
      log_(Fwk::Log::LogNew("ARPQueueDaemon")) { }


void ARPQueueDaemon::run() {
  ARPQueue::Ptr queue = cp_->arpQueue();
  DataPlane::Ptr dp = cp_->dataPlane();

  // Entries to be removed.
  vector<ARPQueue::Entry::Ptr> remove;

  // TODO(ms): need locks here.

  for (ARPQueue::iterator it = queue->begin(); it != queue->end(); ++it) {
    ARPQueue::Entry::Ptr entry = it->second;
    if (entry->retries() >= kMaxRetries) {
      remove.push_back(entry);
      continue;
    }

    DLOG << "resending ARP request for " << entry->ipAddr();
    dp->outputPacketNew(entry->request(), entry->interface());

    entry->retriesInc();
  }

  // Remove entries that have exceeded the maximum number of retries.
  vector<ARPQueue::Entry::Ptr>::iterator it;
  for (it = remove.begin(); it != remove.end(); ++it) {
    DLOG << "Retry count for ARP queue for " << (*it)->ipAddr() << " exceeded";

    // Send ICMP messages to all packets in queue.
    ARPQueue::PacketWrapper::Ptr pkt_wrapper = (*it)->front();
    EthernetPacket::Ptr eth_pkt;
    while (pkt_wrapper != NULL) {
      eth_pkt = pkt_wrapper->packet();
      if (eth_pkt->type() != EthernetPacket::kIP)
        continue;
      IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(eth_pkt->payload());

      cp_->sendICMPDestNetworkUnreach(ip_pkt);
      pkt_wrapper = pkt_wrapper->next();
    }

    // Remove the ARP queue entry.
    queue->entryDel(*it);
  }
}
