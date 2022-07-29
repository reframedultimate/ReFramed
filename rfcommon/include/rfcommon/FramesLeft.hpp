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

    Type value() const;
    double secondsLeft() const;

    bool operator==(FramesLeft rhs) const;
    bool operator!=(FramesLeft rhs) const;

private:
    FramesLeft();
    FramesLeft(Type value);

private:
    Type value_;
};

}
