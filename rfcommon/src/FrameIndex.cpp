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
FrameIndex::~FrameIndex()
{}

// ----------------------------------------------------------------------------
FrameIndex::FrameIndex(Type value)
    : value_(value)
{}

}
