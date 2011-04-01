#ifndef BUFFER_H_QNX4VITL
#define BUFFER_H_QNX4VITL

#include <cstdlib>
#include <inttypes.h>
#include <string.h>

#include "fwk/ptr_interface.h"


class Buffer : public Fwk::PtrInterface<Buffer> {
 public:
  typedef Fwk::Ptr<const Buffer> PtrConst;
  typedef Fwk::Ptr<Buffer> Ptr;

  static Ptr BufferNew(void* buffer, size_t len) {
    return new Buffer(buffer, len);
  }

  uint8_t* data() const { return buffer_; }
  size_t len() const { return len_; }

 protected:
  Buffer(void* buffer, size_t len) {
    buffer_ = new uint8_t[len];
    memcpy(buffer_, buffer, len);
    len_ = len;
  }

  ~Buffer() {
    delete [] buffer_;
  }

 private:
  uint8_t* buffer_;
  size_t len_;

  /* Operations disallowed. */
  Buffer(const Buffer&);
  void operator=(const Buffer&);
};

#endif
