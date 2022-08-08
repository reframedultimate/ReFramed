#include "rfcommon/FrameIndex.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FrameIndex FrameIndex::fromValue(Type value)
{
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
