#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API FrameIndex
{
public:
    typedef uint32_t Type;

    static FrameIndex fromValue(Type value);

    ~FrameIndex();

    Type index() const { return value_; }
    double secondsPassed() const { return static_cast<double>(value_) / 60.0; }

    bool operator==(FrameIndex other) const { return value_ == other.value_; }
    bool operator!=(FrameIndex other) const { return value_ != other.value_; }

private:
    FrameIndex(Type value);

private:
    Type value_;
};

}
