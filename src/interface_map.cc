#include "interface_map.h"

#include <string>
#include <map>

#include "fwk/exception.h"
#include "fwk/notifier.h"
#include "interface.h"
#include "ip_packet.h"

using std::string;

const size_t InterfaceMap::kMaxInterfaces = 32;


InterfaceMap::InterfaceMap()
    : Fwk::BaseNotifier<InterfaceMap, InterfaceMapNotifiee>("InterfaceMap"),
      next_index_(0),
      iface_reactor_(InterfaceReactor::New(this)) { }


void InterfaceMap::interfaceIs(Interface::Ptr iface) {
  if (!iface)
    throw Fwk::ResourceException("InterfaceMap::interfaceIs",
                                 "Interface is NULL");
  if (name_if_map_.find(iface->name()) != name_if_map_.end())
    return;

  iface_reactor_->notifierIs(iface);

  // Set the index and increase the next available index.
  iface->indexIs(next_index_++);

  name_if_map_[iface->name()] = iface;
  ip_if_map_[iface->ip()] = iface;

  // Dispatch notification.
  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onInterface(this, iface);
}


void InterfaceMap::interfaceDel(const std::string& name) {
  Interface::Ptr iface = interface(name);
  interfaceDel(iface);
}


void InterfaceMap::interfaceDel(const Interface::Ptr iface) {
  // TODO(ms): Throw an exception here instead?
  if (!iface)
    return;
  if (name_if_map_.find(iface->name()) == name_if_map_.end())
    return;

  iface_reactor_->notifierDel(iface);

  name_if_map_.erase(iface->name());
  ip_if_map_.erase(iface->ip());

  // Rearrange the indices.
  unsigned int new_next_index = 0;
  for (NameInterfaceMap::iterator it = begin(); it != end(); ++it) {
    Interface::Ptr it_iface = it->second;
    // If we are not deleting the max index interface, swap the max index and
    // the deleted index. This way, our indices are always continguous.
    if (iface->index() < next_index_ - 1 &&
        it_iface->index() == next_index_ - 1)
      it_iface->indexIs(iface->index());

    // Find new next available index.
    if (it_iface->index() >= new_next_index)
      new_next_index = it_iface->index() + 1;
  }
  next_index_ = new_next_index;

  // Dispatch notification.
  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onInterfaceDel(this, iface);
}


Interface::Ptr InterfaceMap::interface(const string& name) {
  NameInterfaceMap::iterator it = name_if_map_.find(name);
  return (it != name_if_map_.end()) ? it->second : NULL;
}


Interface::Ptr InterfaceMap::interfaceAddr(const IPv4Addr& addr) {
  IPInterfaceMap::iterator it = ip_if_map_.find(addr);
  return (it != ip_if_map_.end()) ? it->second : NULL;
}


// Forward interface notifications.

void
InterfaceMap::InterfaceReactor::onIP(Interface::Ptr iface) {
  for (unsigned int i = 0; i < iface_map_->notifiees_.size(); ++i)
    iface_map_->notifiees_[i]->onInterfaceIP(iface_map_, iface);
}

void
InterfaceMap::InterfaceReactor::onMAC(Interface::Ptr iface) {
  for (unsigned int i = 0; i < iface_map_->notifiees_.size(); ++i)
    iface_map_->notifiees_[i]->onInterfaceMAC(iface_map_, iface);
}

void
InterfaceMap::InterfaceReactor::onEnabled(Interface::Ptr iface) {
  for (unsigned int i = 0; i < iface_map_->notifiees_.size(); ++i)
    iface_map_->notifiees_[i]->onInterfaceEnabled(iface_map_, iface);
}
