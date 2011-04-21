#ifndef TIME_TYPES_H
#define TIME_TYPES_H

#include <inttypes.h>
#include <time.h>

#include "fwk/ordinal.h"


class Seconds : public Fwk::Ordinal<Seconds, uint32_t> {
 public:
  Seconds(const uint32_t s) : Ordinal<Seconds, uint32_t>(s) { }
};


class TimeEpoch : public Fwk::Ordinal<TimeEpoch, time_t> {
 public:
  TimeEpoch(const time_t t) : Ordinal<TimeEpoch, time_t>(t) { }
};

#endif
