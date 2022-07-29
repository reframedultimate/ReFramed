#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API FrameNumber
{
public:
    typedef uint32_t Type;

    static FrameNumber fromValue(Type value);

    ~FrameNumber();

    Type value() const;
    double secondsPassed() const;

    bool operator==(FrameNumber other) const;
    bool operator!=(FrameNumber other) const;

private:
    FrameNumber();
    FrameNumber(Type value);

private:
    Type value_;
};

}
