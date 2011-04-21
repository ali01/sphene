#ifndef TIME_TYPES_H
#define TIME_TYPES_H

#include <inttypes.h>
#include <time.h>

#include "fwk/ordinal.h"


class Seconds : public Fwk::Ordinal<Seconds, uint32_t> { };
class TimeEpoch : public Fwk::Ordinal<TimeEpoch, time_t> { };

#endif
