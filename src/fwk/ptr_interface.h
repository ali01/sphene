/* Adapted from David R. Cheriton's Advanced Object Oriented Programming from a
  Modeling & Simulation's Perspective ~ Chapter 5: Memory management with smart
  pointers. */

#ifndef PTRINTERFACE_H_VGPMQRTI
#define PTRINTERFACE_H_VGPMQRTI

/* stl includes */
#include <inttypes.h>
#include "atomic.h"
#include "ptr.h"

namespace Fwk {

template <typename T>
class PtrInterface {
 public:
  uint64_t references() const {
    return ref_;
  }

  virtual const PtrInterface * newRef() const {
    ++ref_;
    return this;
  }

  virtual const PtrInterface * deleteRef() const {
    if (--ref_ == 0) {
      onZeroReferences();
      return 0;
    }

    return this;
  }

 protected:
  PtrInterface() : ref_(0) {}
  virtual ~PtrInterface() {}
  virtual void onZeroReferences() const { delete this; }

  mutable AtomicUInt64 ref_;

 private:
  friend class Ptr<T>;
};

} /* end of namespace ptr_interface */

#endif
