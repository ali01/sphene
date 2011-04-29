#include "tunnel_map.h"

#include <string>
#include <map>

#include "ipv4_addr.h"
#include "fwk/exception.h"
#include "tunnel.h"

using std::string;


void TunnelMap::tunnelIs(const Tunnel::Ptr tunnel) {
  if (!tunnel)
    throw Fwk::ResourceException("TunnelMap::tunnelIs",
                                 "Tunnel is NULL");

  name_t_map_[tunnel->name()] = tunnel;
  ip_t_map_[tunnel->remote()] = tunnel;
}


void TunnelMap::tunnelDel(const std::string& name) {
  Tunnel::Ptr t = tunnel(name);
  tunnelDel(t);
}


void TunnelMap::tunnelDel(Tunnel::Ptr tunnel) {
  // TODO(ms): Throw an exception here instead?
  if (!tunnel)
    return;

  name_t_map_.erase(tunnel->name());
  ip_t_map_.erase(tunnel->remote());
}


Tunnel::Ptr TunnelMap::tunnel(const string& name) {
  NameTunnelMap::iterator it = name_t_map_.find(name);
  return (it != name_t_map_.end()) ? it->second : NULL;
}


Tunnel::Ptr TunnelMap::tunnelRemoteAddr(const IPv4Addr& addr) {
  IPTunnelMap::iterator it = ip_t_map_.find(addr);
  return (it != ip_t_map_.end()) ? it->second : NULL;
}
