#include "router.h"

#include <string>

#include "control_plane.h"
#include "data_plane.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "task.h"

using std::string;


Router::Router(const string& name,
               ControlPlane::Ptr cp,
               DataPlane::Ptr dp,
               TaskManager::Ptr tm)
    : Fwk::NamedInterface(name),
      cp_(cp),
      dp_(dp),
      log_(Fwk::Log::LogNew(name)),
      tm_(tm) {
  DLOG << "Initializing";

  // Initialize pointers between ControlPlane and DataPlane.
  cp->dataPlaneIs(dp);
  dp->controlPlaneIs(cp.ptr());  // weak pointer to prevent circular reference

  // Set RoutingTable pointer in DataPlane.
  dp->routingTableIs(cp->routingTable());
}


Router::~Router() {
  DLOG << "Destroying";
}
