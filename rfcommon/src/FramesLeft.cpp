#include "rfcommon/FramesLeft.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FramesLeft FramesLeft::fromValue(Type value)
{
    return FramesLeft(value);
}

// ----------------------------------------------------------------------------
FramesLeft::~FramesLeft()
{

}

// ----------------------------------------------------------------------------
FramesLeft::Type FramesLeft::value() const
{ 
    return value_;
}

// ----------------------------------------------------------------------------
double FramesLeft::secondsLeft() const
{ 
    return static_cast<double>(value_) / 60.0; 
}

// ----------------------------------------------------------------------------
bool FramesLeft::operator==(FramesLeft rhs) const 
{
    return value_ == rhs.value_;
}

// ----------------------------------------------------------------------------
bool FramesLeft::operator!=(FramesLeft rhs) const
{ 
    return value_ != rhs.value_; 
}

// ----------------------------------------------------------------------------
FramesLeft::FramesLeft()
    : value_(0)
{}

// ----------------------------------------------------------------------------
FramesLeft::FramesLeft(Type value) 
    : value_(value) 
{}

}
