#include "rfcommon/TimeStamp.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
TimeStamp TimeStamp::fromMillisSinceEpoch(Type value)
{
    return TimeStamp(value);
}

// ----------------------------------------------------------------------------
TimeStamp TimeStamp::makeInvalid()
{
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
