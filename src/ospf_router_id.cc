#include "ospf_router_id.h"

#include <string>

#include "ipv4_addr.h"

RouterID::operator std::string() const {
  return IPv4Addr(this->value());
}

std::ostream&
operator<<(std::ostream& out, const RouterID& rid) {
  return out << (std::string)IPv4Addr(rid.value());
}
