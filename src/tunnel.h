#ifndef TUNNEL_H_
#define TUNNEL_H_

#include "fwk/named_interface.h"
#include "fwk/ptr.h"

#include "interface.h"
#include "ipv4_addr.h"


class Tunnel : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const Tunnel> PtrConst;
  typedef Fwk::Ptr<Tunnel> Ptr;

  enum Mode {
    kGRE
  };

  static Ptr New(Interface::PtrConst iface) {
    return new Tunnel(iface);
  }

  // Returns the interface for this tunnel.
  Interface::PtrConst interface() const { return iface_; }

  // Returns the remote endpoint for this tunnel.
  IPv4Addr remote() const { return remote_; }

  // Sets the remote endpoint for this tunnel.
  void remoteIs(IPv4Addr remote) { remote_ = remote; }

  // Returns the mode of this tunnel.
  Mode mode() const { return mode_; }

  // Sets the mode of this tunnel.
  void modeIs(Mode mode) { mode_ = mode; }

 protected:
  Tunnel(Interface::PtrConst iface);

  Interface::PtrConst iface_;
  IPv4Addr remote_;
  Mode mode_;

 private:
  Tunnel(const Tunnel&);
  void operator=(const Tunnel&);
};

#endif
