#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API FramesLeft
{
public:
    typedef uint32_t Type;

    static FramesLeft fromValue(Type value);

    ~FramesLeft();

    Type count() const { return value_; }
    double secondsLeft() const { return static_cast<double>(value_) / 60.0; }

    bool operator==(FramesLeft rhs) const { return value_ == rhs.value_; }
    bool operator!=(FramesLeft rhs) const { return value_ != rhs.value_; }
    bool operator<(FramesLeft rhs) const { return value_ < rhs.value_; }
    bool operator>(FramesLeft rhs) const { return value_ > rhs.value_; }
    bool operator<=(FramesLeft rhs) const { return value_ <= rhs.value_; }
    bool operator>=(FramesLeft rhs) const { return value_ >= rhs.value_; }

private:
    FramesLeft(Type value);

private:
    Type value_;
};

}
