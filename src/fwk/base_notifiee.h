#ifndef BASE_NOTIFIEE_H_BKB8TWOG
#define BASE_NOTIFIEE_H_BKB8TWOG

#include "ptr_interface.h"

namespace Fwk {

template <typename Notifier>
class BaseNotifiee : public PtrInterface<BaseNotifiee<Notifier> > {
 public:
  typedef Fwk::Ptr<const BaseNotifiee<Notifier> > PtrConst;
  typedef Fwk::Ptr<BaseNotifiee<Notifier> > Ptr;

  typename Notifier::PtrConst notifier() const { return notifier_; }
  typename Notifier::Ptr notifier() { return notifier_; }

 protected:
  BaseNotifiee(typename Notifier::Ptr _n) : notifier_(_n) {}
  virtual ~BaseNotifiee() {}

 private:
   /* Data members. */
  typename Notifier::Ptr notifier_;

  /* Operations disallowed. */
  BaseNotifiee(const BaseNotifiee&);
  void operator=(const BaseNotifiee&);
};

} /* end of namespace Fwk */

#endif
