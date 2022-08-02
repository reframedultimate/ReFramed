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
TimeStamp::Type TimeStamp::millisSinceEpoch() const
{
    return millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
bool TimeStamp::isValid() const
{
    return millisSinceEpoch_ != 0;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator==(TimeStamp rhs) const
{
    return millisSinceEpoch_ == rhs.millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator!=(TimeStamp rhs) const
{
    return millisSinceEpoch_ != rhs.millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator<(TimeStamp rhs) const
{
    return millisSinceEpoch_ < rhs.millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator<=(TimeStamp rhs) const
{
    return millisSinceEpoch_ <= rhs.millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator>(TimeStamp rhs) const
{
    return millisSinceEpoch_ > rhs.millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
bool TimeStamp::operator>=(TimeStamp rhs) const
{
    return millisSinceEpoch_ >= rhs.millisSinceEpoch_;
}

// ----------------------------------------------------------------------------
TimeStamp& TimeStamp::operator+=(DeltaTime rhs)
{
    millisSinceEpoch_ += rhs.millis();
    return *this;
}

// ----------------------------------------------------------------------------
TimeStamp& TimeStamp::operator-=(DeltaTime rhs)
{
    millisSinceEpoch_ -= rhs.millis();
    return *this;
}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp()
    : millisSinceEpoch_(0)
{}

// ----------------------------------------------------------------------------
TimeStamp::TimeStamp(Type millisSinceEpoch)
    : millisSinceEpoch_(millisSinceEpoch)
{}

}
