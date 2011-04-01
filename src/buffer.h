#ifndef BUFFER_H_QNX4VITL
#define BUFFER_H_QNX4VITL

#include "fwk/ptr_interface.h"

class Buffer : public Fwk::PtrInterface<Buffer> {
 public:
  typedef Fwk::Ptr<const Buffer> PtrConst;
  typedef Fwk::Ptr<Buffer> Ptr;

  static Ptr BufferNew(void* buffer, size_t len) {
    return new Buffer(buffer, len);
  }

  void* data() const { return buffer_; }
  size_t len() const { return len_; }

 protected:
  Buffer(void* buffer, size_t len) : buffer_(buffer), len_(len) { }

  /* data members */
  void* buffer_;
  size_t len_;

  /* operations disallowed */
  Buffer(const Buffer&);
  void operator=(const Buffer&);
};

#endif
