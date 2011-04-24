#include "router.h"

#include <string>

#include "control_plane.h"
#include "data_plane.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"

using std::string;


Router::Router(const string& name,
               ControlPlane::Ptr cp,
               DataPlane::Ptr dp)
    : Fwk::NamedInterface(name),
      cp_(cp),
      dp_(dp),
      log_(Fwk::Log::LogNew(name)) {
  DLOG << "Initializing";

  // Initialize pointers between ControlPlane and DataPlane.
  cp->dataPlaneIs(dp);
  dp->controlPlaneIs(cp.ptr());  // weak pointer to prevent circular reference
}
