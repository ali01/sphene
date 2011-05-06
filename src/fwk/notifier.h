#ifndef FWK_NOTIFIER_H_
#define FWK_NOTIFIER_H_

#include <vector>

#include "ptr.h"
#include "ptr_interface.h"

namespace Fwk {

class Notifier;

class Notifiee : public PtrInterface<Notifiee> {
 public:
  virtual void notifierIs(Ptr<Notifier> notifier);

 protected:
  Notifiee();

  Notifier* notifier_;
};


class Notifier : public PtrInterface<Notifier> {
 protected:
  virtual void notifieeIs(Ptr<Notifiee> notifiee);
  virtual void notifieeDel(Ptr<Notifiee> notifiee);

  std::vector<Ptr<Notifiee> > notifiees_;

 private:
  friend class Notifiee;
};

}  // Fwk

#endif
