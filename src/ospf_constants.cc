#include "ospf_constants.h"

namespace OSPF {

const uint8_t kDefaultLinkStateInterval = 10;
const uint8_t kDefaultHelloInterval = 5;
const uint8_t kDefaultLinkStateUpdateTimeout = 3 * kDefaultLinkStateInterval;
const uint32_t kLinkStateWindow = 10;
const AreaID kDefaultAreaID = 0;
const RouterID kPassiveEndpointID = 0;
const RouterID kInvalidRouterID = 0xffffffff;

} /* end of namespace OSPF */
