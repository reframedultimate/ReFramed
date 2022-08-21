#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FramesLeft FramesLeft::fromValue(Type value)
{
    NOPROFILE();

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
