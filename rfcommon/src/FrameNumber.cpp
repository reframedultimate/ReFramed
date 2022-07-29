#include "rfcommon/FrameNumber.hpp"

#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

// ----------------------------------------------------------------------------
FrameNumber FrameNumber::fromValue(Type value)
{
    return FrameNumber(value);
}

// ----------------------------------------------------------------------------
FrameNumber::~FrameNumber()
{}

// ----------------------------------------------------------------------------
FrameNumber::Type FrameNumber::value() const 
{
    return value_; 
}

// ----------------------------------------------------------------------------
double FrameNumber::secondsPassed() const
{ 
    return static_cast<double>(value_) / 60.0; 
}

// ----------------------------------------------------------------------------
bool FrameNumber::operator==(FrameNumber other) const
{ 
    return value_ == other.value_; 
}

// ----------------------------------------------------------------------------
bool FrameNumber::operator!=(FrameNumber other) const 
{
    return value_ != other.value_;
}

// ----------------------------------------------------------------------------
FrameNumber::FrameNumber() 
    : value_(0)
{}

// ----------------------------------------------------------------------------
FrameNumber::FrameNumber(Type value) 
    : value_(value) 
{}

}
