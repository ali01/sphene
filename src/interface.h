#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <inttypes.h>
#include <string>

#include "fwk/notifier.h"
#include "fwk/ptr.h"

#include "ethernet_packet.h"
#include "ip_packet.h"


class InterfaceNotifiee;

class Interface
    : public Fwk::BaseNotifier<Interface, InterfaceNotifiee> {
 public:
  typedef Fwk::Ptr<const Interface> PtrConst;
  typedef Fwk::Ptr<Interface> Ptr;
  typedef InterfaceNotifiee Notifiee;

  enum Type {
    kHardware,
    kVirtual
  };

  static Ptr InterfaceNew(const std::string& name) {
    return new Interface(name);
  }

  // Returns hardware (MAC) address.
  EthernetAddr mac() const { return mac_; }

  // Sets the hardware (MAC) address.
  void macIs(const EthernetAddr& addr);

  // Returns IPv4 address.
  IPv4Addr ip() const { return ip_; }

  // Sets the interface IP address.
  void ipIs(const IPv4Addr& ip);

  // Returns subnet mask.
  IPv4Addr subnetMask() const { return mask_; }

  // Sets the interface subnet mask.
  void subnetMaskIs(const IPv4Addr& mask) { mask_ = mask; }

  // Returns interface speed.
  // TODO(ms): better type for this?
  uint32_t speed() const { return speed_; }

  // Sets the interface speed.
  void speedIs(uint32_t speed) { speed_ = speed; }

  // Returns whether or not the interface is enabled.
  bool enabled() const { return enabled_; }

  // Sets the enabled flag.
  void enabledIs(bool enabled) { enabled_ = enabled; }

  // Returns the type of interface.
  Type type() const { return type_; }

  // Sets the interface type.
  void typeIs(Type type) { type_ = type; }

  // Returns the socket descriptor for this interface.
  int socketDescriptor() const { return socket_; }

  // Sets the socket descriptor.
  void socketDescriptorIs(int s) { socket_ = s; }

  // Returns the index.
  unsigned int index() const { return index_; }

  // Sets the index. This is usually called by a container (e.g.,
  // InterfaceMap).
  void indexIs(unsigned int index) { index_ = index; }

 protected:
  Interface(const std::string& name);

  bool enabled_;
  EthernetAddr mac_;
  IPv4Addr ip_;
  IPv4Addr mask_;
  uint32_t speed_;
  Type type_;
  int socket_;
  unsigned int index_;

 private:
  Interface(const Interface&);
  void operator=(const Interface&);
};


class InterfaceNotifiee
    : public Fwk::BaseNotifiee<Interface, InterfaceNotifiee> {
 public:
  virtual void onIP(Interface::Ptr iface) { }
  virtual void onMAC(Interface::Ptr iface) { }
};

#endif
