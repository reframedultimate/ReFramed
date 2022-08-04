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
FrameIndex::Type FrameIndex::value() const
{
    return value_;
}

// ----------------------------------------------------------------------------
double FrameIndex::secondsPassed() const
{
    return static_cast<double>(value_) / 60.0;
}

// ----------------------------------------------------------------------------
bool FrameIndex::operator==(FrameIndex other) const
{
    return value_ == other.value_;
}

// ----------------------------------------------------------------------------
bool FrameIndex::operator!=(FrameIndex other) const
{
    return value_ != other.value_;
}

// ----------------------------------------------------------------------------
FrameIndex::FrameIndex()
    : value_(0)
{}

// ----------------------------------------------------------------------------
FrameIndex::FrameIndex(Type value)
    : value_(value)
{}

}
