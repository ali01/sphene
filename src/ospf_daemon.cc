#include "ospf_daemon.h"

#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "control_plane.h"
#include "data_plane.h"
#include "ospf_router.h"
#include "task.h"
#include "time_types.h"


/* Static global log instance */
static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("OSPFDaemon");


OSPFDaemon::Ptr
OSPFDaemon::New(OSPFRouter::Ptr ospf_router,
                ControlPlane::Ptr cp,
                DataPlane::Ptr dp) {
  return new OSPFDaemon(ospf_router, cp, dp);
}

OSPFDaemon::OSPFDaemon(OSPFRouter::Ptr ospf_router,
                       ControlPlane::Ptr cp,
                       DataPlane::Ptr dp)
    : PeriodicTask("OSPFDaemon"),
      control_plane_(cp),
      data_plane_(dp),
      ospf_router_(ospf_router) {}

void
OSPFDaemon::run() {
  // TODO(ms): This runs every second. Send Hello packets and LSU packets here
  //   as necessary. Also timeout any neighbors or routers in the topology that
  //   have not sent Hello or LSU packets within the required time period.
}
