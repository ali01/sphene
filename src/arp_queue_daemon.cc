#include "arp_queue_daemon.h"

#include <vector>

#include "arp_queue.h"
#include "control_plane.h"
#include "fwk/log.h"
#include "task.h"
#include "time_types.h"

static const Seconds kMaxRetries = 5;

using std::vector;


ARPQueueDaemon::ARPQueueDaemon(ControlPlane::Ptr cp)
    : PeriodicTask("ARPQueueDaemon"),
      cp_(cp),
      log_(Fwk::Log::LogNew("ARPQueueDaemon")) { }


void ARPQueueDaemon::run() {
  DLOG << "ARP queue daemon running";
}
