#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API DeltaTime
{
public:
    typedef uint64_t Type;

    static DeltaTime fromMillis(Type value);

    ~DeltaTime();

    Type millis() const { return value_; }
    Type seconds() const { return value_ / 1000; }

private:
    DeltaTime();
    DeltaTime(Type value);

private:
    Type value_;
};

}
