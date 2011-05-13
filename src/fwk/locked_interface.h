#ifndef FWK_LOCKED_INTERFACE_H_
#define FWK_LOCKED_INTERFACE_H_

#include <pthread.h>

namespace Fwk {

// LockedInterface provides a coarse lock for a derived class. Derived classes
// should use Fwk::ScopedLock when possible.
class LockedInterface {
 public:
  LockedInterface() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    // Lock is recursive.
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&li_lock_, &attr);

    pthread_mutexattr_destroy(&attr);
  }

  void lockedIs(bool locked) {
    if (locked)
      pthread_mutex_lock(&li_lock_);
    else
      pthread_mutex_unlock(&li_lock_);
  }

 private:
  pthread_mutex_t li_lock_;
};

};

#endif
