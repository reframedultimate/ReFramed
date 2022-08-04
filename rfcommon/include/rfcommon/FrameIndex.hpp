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

    Type value() const;
    double secondsPassed() const;

    bool operator==(FrameIndex other) const;
    bool operator!=(FrameIndex other) const;

private:
    FrameIndex();
    FrameIndex(Type value);

private:
    Type value_;
};

}
