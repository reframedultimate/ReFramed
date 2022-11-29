#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/Profiler.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
TimeStamp TimeStamp::fromMillisSinceEpoch(Type value)
{
    NOPROFILE();

    return TimeStamp(value);
}

// ----------------------------------------------------------------------------
TimeStamp TimeStamp::fromSecondsSinceEpoch(Type value)
{
    NOPROFILE();

    return TimeStamp(value * 1000);
}

// ----------------------------------------------------------------------------
TimeStamp TimeStamp::makeInvalid()
{
    NOPROFILE();

    return TimeStamp(0);
}

// ----------------------------------------------------------------------------
TimeStamp::~TimeStamp()
{}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp(Type millisSinceEpoch)
    : millisSinceEpoch_(millisSinceEpoch)
{}

}
