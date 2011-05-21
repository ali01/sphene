#ifndef OSPF_CONSTANTS_H_BQNRNDUY
#define OSPF_CONSTANTS_H_BQNRNDUY

#include <inttypes.h>

#include "ospf_types.h"

namespace OSPF {

/* Interval for OSPF LSU flood updates. */
extern const uint8_t kDefaultLinkStateInterval;
extern const uint8_t kDefaultHelloInterval;
extern const uint8_t kDefaultLinkStateUpdateTimeout;
extern const uint32_t kLinkStateWindow;
extern const AreaID kDefaultAreaID;
extern const RouterID kPassiveEndpointID;
extern const RouterID kInvalidRouterID;

} /* end of namespace OSPF */

#endif
