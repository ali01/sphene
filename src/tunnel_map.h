#ifndef TUNNEL_MAP_H_
#define TUNNEL_MAP_H_

#include <map>
#include <string>

#include "ipv4_addr.h"
#include "fwk/locked_interface.h"
#include "fwk/notifier.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"
#include "tunnel.h"


class TunnelMapNotifiee;

class TunnelMap : public Fwk::BaseNotifier<TunnelMap, TunnelMapNotifiee>,
                  public Fwk::LockedInterface {
 public:
  typedef Fwk::Ptr<const TunnelMap> PtrConst;
  typedef Fwk::Ptr<TunnelMap> Ptr;
  typedef TunnelMapNotifiee Notifiee;
  typedef std::map<std::string, Tunnel::Ptr> NameTunnelMap;
  typedef std::map<IPv4Addr, Tunnel::Ptr> IPTunnelMap;
  typedef NameTunnelMap::iterator iterator;
  typedef NameTunnelMap::const_iterator const_iterator;

  static Ptr New() {
    return new TunnelMap();
  }

  // Adds a new tunnel to the map. If a tunnel with the same name already
  // exists, it is replaced with 'iface'.
  void tunnelIs(Tunnel::Ptr tunnel);

  // Delete a tunnel.
  void tunnelDel(const std::string& name);
  void tunnelDel(Tunnel::Ptr tunnel);

  // Returns a tunnel by name 'name'. Returns NULL if no tunnel exists by that
  // name.
  Tunnel::Ptr tunnel(const std::string& name);

  // Returns a tunnel with remote IP address 'addr'. Returns NULL if no tunnel
  // exists with that remote address.
  Tunnel::Ptr tunnelRemoteAddr(const IPv4Addr& addr);

  // Returns the number of tunnels in the map.
  size_t tunnels() const { return name_t_map_.size(); }

  iterator begin() { return name_t_map_.begin(); }
  iterator end() { return name_t_map_.end(); }
  const_iterator begin() const { return name_t_map_.begin(); }
  const_iterator end() const { return name_t_map_.end(); }

 protected:
  TunnelMap() { }

 private:
  NameTunnelMap name_t_map_;
  IPTunnelMap ip_t_map_;

  TunnelMap(const TunnelMap&);
  void operator=(const TunnelMap&);
};


class TunnelMapNotifiee
    : public Fwk::BaseNotifiee<TunnelMap, TunnelMapNotifiee> {
 public:
  virtual void onTunnel(TunnelMap::Ptr tunnel_map,
                        Tunnel::Ptr tunnel) { }
  virtual void onTunnelDel(TunnelMap::Ptr rtable,
                           Tunnel::Ptr tunnel) { }
};

#endif
