#ifndef SCOPED_LOCK_H_P141WST0
#define SCOPED_LOCK_H_P141WST0

#include "ptr.h"

namespace Fwk {

template <typename T>
class ScopedLock {
 public:
  ScopedLock(Ptr<T> object) : object_(object) {
    object->lockedIs(true);
  }

  ~ScopedLock() {
    object_->lockedIs(false);
  }

 private:
  Ptr<T> object_;
};

} /* end of namespace Fwk */

#endif
