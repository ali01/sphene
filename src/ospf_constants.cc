#include "ospf_constants.h"

namespace OSPF {

const uint8_t kDefaultLinkStateInterval = 30;
const uint8_t kDefaultHelloInterval = 10;
const uint8_t kDefaultLSUTimeout = 3 * kDefaultLinkStateInterval;
const AreaID kDefaultAreaID = 0;
const RouterID kInvalidRouterID = 0xffffffff;

} /* end of namespace OSPF */
