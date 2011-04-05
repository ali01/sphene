#include "packet.h"

#include "fwk/exception.h"

uint8_t*
Packet::offsetAddress(unsigned int offset) const {
  if (buffer_offset_ + offset >= buffer_->len())
    throw Fwk::RangeException("offsetAddress", "Buffer index out of bounds");

  return buffer_->data() + buffer_offset_ + offset;
}
