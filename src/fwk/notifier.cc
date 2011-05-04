#include "notifier.h"

#include <vector>

#include "ptr.h"

namespace Fwk {

Notifiee::Notifiee() : notifier_(NULL) { }


void Notifiee::notifierIs(Ptr<Notifier> notifier) {
  // Tell old notifier.
  if (notifier_)
    notifier_->notifieeDel(this);

  // Tell new notifier.
  if (notifier) {
    notifier_ = notifier.ptr();
    notifier_->notifieeIs(this);
  }
}


void Notifier::notifieeIs(Ptr<Notifiee> notifiee) {
  for (unsigned int i = 0; i < notifiees_.size(); ++i) {
    if (notifiees_[i] == notifiee)
      return;  // notifiee already exists.
  }

  notifiees_.push_back(notifiee);
}


void Notifier::notifieeDel(Ptr<Notifiee> notifiee) {
  for (unsigned int i = 0; i < notifiees_.size(); ++i) {
    if (notifiees_[i] == notifiee) {
      notifiees_.erase(notifiees_.begin() + i);
      return;
    }
  }
}

}
