#include "ospf_daemon.h"

#include <vector>

#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "control_plane.h"
#include "ospf_router.h"
#include "task.h"
#include "time_types.h"

using std::vector;


OSPFDaemon::OSPFDaemon(OSPFRouter::Ptr ospf_rtr, ControlPlane::Ptr cp)
    : PeriodicTask("OSPFDaemon"),
      cp_(cp),
      ospf_rtr_(ospf_rtr),
      log_(Fwk::Log::LogNew("OSPFDaemon")) { }


void OSPFDaemon::run() {
  // TODO(ms): This runs every second. Send Hello packets and LSU packets here
  //   as necessary. Also timeout any neighbors or routers in the topology that
  //   have not sent Hello or LSU packets within the required time period.
}
