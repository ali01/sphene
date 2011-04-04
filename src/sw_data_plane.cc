#include "sw_data_plane.h"

#include "fwk/log.h"


SWDataPlane::SWDataPlane() : DataPlane("SWDataPlane") {
  log_ = Fwk::Log::LogNew("SWDataPlane");

  log_->entryNew("constructor");
}
