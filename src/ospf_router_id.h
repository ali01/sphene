#ifndef OSPF_ROUTER_ID_H_YZIV60KY
#define OSPF_ROUTER_ID_H_YZIV60KY

#include <inttypes.h>
#include <ostream>
#include <string>

#include "fwk/ordinal.h"

class RouterID : public Fwk::Ordinal<RouterID,uint32_t> {
 public:
  RouterID() : Fwk::Ordinal<RouterID,uint32_t>(0) {}
  RouterID(uint32_t rid) : Fwk::Ordinal<RouterID,uint32_t>(rid) {}

  operator std::string() const;
};

std::ostream& operator<<(std::ostream& out, const RouterID& rid);

#endif
