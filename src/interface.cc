#include "interface.h"

#include <string>

#include "fwk/notifier.h"

using std::string;


Interface::Interface(const string& name)
    : Fwk::BaseNotifier<Interface, InterfaceNotifiee>(name),
      enabled_(true),
      speed_(0),
      type_(kHardware),
      socket_(-1) { }


void Interface::macIs(const EthernetAddr& addr) {
  if (mac_ == addr)
    return;

  mac_ = addr;

  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onMAC(this);
}


void Interface::ipIs(const IPv4Addr& ip) {
  if (ip_ == ip)
    return;

  ip_ = ip;

  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onIP(this);
}


void Interface::enabledIs(const bool enabled) {
  if (enabled_ == enabled)
    return;

  enabled_ = (enabled > 0);

  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onEnabled(this);
}
