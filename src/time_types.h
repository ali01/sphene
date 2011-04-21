#ifndef TIME_TYPES_H
#define TIME_TYPES_H

#include <inttypes.h>
#include <time.h>

#include "fwk/ordinal.h"


class Seconds : public Fwk::Ordinal<Seconds, int32_t> {
 public:
  Seconds(const int32_t s) : Ordinal<Seconds, int32_t>(s) { }
};


class TimeDelta : public Fwk::Ordinal<TimeDelta, time_t> {
 public:
  TimeDelta(const time_t d) : Ordinal<TimeDelta, time_t>(d) { }
  operator Seconds() const { return Seconds(value()); }
};


class TimeEpoch : public Fwk::Ordinal<TimeEpoch, time_t> {
 public:
  TimeEpoch(const time_t t) : Ordinal<TimeEpoch, time_t>(t) { }
  TimeDelta operator-(const TimeEpoch& other) const {
    return TimeDelta(value() - other.value());
  }
  operator Seconds() const { return Seconds(value()); }
};

#endif
