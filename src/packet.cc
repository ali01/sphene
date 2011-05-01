#include "packet.h"

#include "fwk/exception.h"

uint8_t*
Packet::offsetAddress(unsigned int offset) const {
  if (bufferOffset() + offset >= buffer_->size())
    throw Fwk::RangeException("offsetAddress", "Buffer index out of bounds");

  return buffer_->data() + bufferOffset() + offset;
}
