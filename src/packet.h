#ifndef PACKET_H_
#define PACKET_H_

#include "buffer.h"

class Packet {
 public:
  Packet(Buffer::Ptr buffer) : buffer_(buffer) { }

  Buffer::Ptr buffer() const { return buffer_; }

 private:
  Buffer::Ptr buffer_;
};

#endif
