#ifndef FWK_NOTIFIER_H_
#define FWK_NOTIFIER_H_

#include <string>
#include <vector>

#include "named_interface.h"
#include "ptr.h"

namespace Fwk {

template <typename Notifier, typename Notifiee>
class BaseNotifiee : public NamedInterface {
 public:
  virtual void notifierIs(Ptr<Notifier> notifier) {
    notifier->notifieeIs(static_cast<Notifiee*>(this));
  }

  virtual void notifierDel(Ptr<Notifier> notifier) {
    notifier->notifieeDel(static_cast<Notifiee*>(this));
  }

 protected:
  BaseNotifiee() : NamedInterface("") { }
  BaseNotifiee(const std::string& name) : NamedInterface(name) { }
  // NOTE(ms): References to the notifier are intentionally not
  //   kept. References to the notifier should be passed as the first argument
  //   to the notification handlers in the notifiee.

 private:
  BaseNotifiee(const BaseNotifiee&);
  void operator=(const BaseNotifiee&);
};


template <typename Notifier, class Notifiee>
class BaseNotifier : public NamedInterface {
 protected:
  BaseNotifier() : NamedInterface("") { }
  BaseNotifier(const std::string& name) : NamedInterface(name) { }

  // Called by Notifiee::notifierIs().
  virtual void notifieeIs(Ptr<Notifiee> notifiee) {
    for (unsigned int i = 0; i < notifiees_.size(); ++i) {
      if (notifiees_[i] == notifiee)
        return;  // notifiee already exists.
    }

    notifiees_.push_back(notifiee);
  }

  // Called by Notifiee::notifierDel().
  virtual void notifieeDel(Ptr<Notifiee> notifiee) {
    for (unsigned int i = 0; i < notifiees_.size(); ++i) {
      if (notifiees_[i] == notifiee) {
        notifiees_.erase(notifiees_.begin() + i);
        return;
      }
    }
  }

  std::vector<Ptr<Notifiee> > notifiees_;

  friend class BaseNotifiee<Notifier, Notifiee>;
};

}  // Fwk

#endif
