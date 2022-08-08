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
FramesLeft::FramesLeft(Type value)
    : value_(value)
{}

}
