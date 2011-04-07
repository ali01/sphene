#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <inttypes.h>
#include <string>

#include "fwk/named_interface.h"
#include "fwk/ptr.h"

#include "ethernet_packet.h"
#include "ip_packet.h"


class Interface : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const Interface> PtrConst;
  typedef Fwk::Ptr<Interface> Ptr;

  static Ptr InterfaceNew(const std::string& name) {
    return new Interface(name);
  }

  // Returns hardware (MAC) address.
  EthernetAddr mac() const { return mac_; }

  // Sets the hardware (MAC) address.
  void macIs(const EthernetAddr& addr) { mac_ = addr; }

  // Returns IPv4 address.
  IPv4Addr ip() const { return ip_; }

  // Sets the interface IP address.
  void ipIs(const IPv4Addr& ip) { ip_ = ip; }

  // Returns subnet mask.
  IPv4Addr subnetMask() const { return mask_; }

  // Sets the interface subnet mask.
  void subnetMaskIs(const IPv4Addr& mask) { mask_ = mask; }

  // Returns interface speed.
  // TODO(ms): better type for this?
  uint32_t speed() const { return speed_; }

  // Sets the interface speed.
  void speedIs(uint32_t speed) { speed_ = speed; }

 protected:
  Interface(const std::string& name);

  EthernetAddr mac_;
  IPv4Addr ip_;
  IPv4Addr mask_;
  uint32_t speed_;

 private:
  Interface(const Interface&);
  void operator=(const Interface&);
};

#endif