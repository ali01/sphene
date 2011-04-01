#include "packet.h"

#include "fwk/exception.h"

uint8_t*
Packet::offsetAddress(unsigned int offset) const {
  if (buffer_offset_ + offset >= buffer_->len()) {
    char *msg = (char *)"Buffer index out of bounds.";
    throw Fwk::RangeException(__FILE__, __LINE__, msg);
  }

  return buffer_->data() + buffer_offset_ + offset;
}
