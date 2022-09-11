#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FrameIndex FrameIndex::fromValue(Type value)
{
    NOPROFILE();

    return FrameIndex(value);
}

// ----------------------------------------------------------------------------
FrameIndex FrameIndex::fromSeconds(double seconds)
{
    NOPROFILE();

    return FrameIndex(static_cast<Type>(seconds * 60.0));
}

// ----------------------------------------------------------------------------
FrameIndex FrameIndex::makeInvalid()
{
    NOPROFILE();

    return FrameIndex(Type(-1));
}

// ----------------------------------------------------------------------------
FrameIndex::~FrameIndex()
{}

// ----------------------------------------------------------------------------
FrameIndex::FrameIndex(Type value)
    : value_(value)
{}

}
