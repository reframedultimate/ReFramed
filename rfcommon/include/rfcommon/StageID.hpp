#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API StageID
{
public:
    typedef uint16_t Type;

    static StageID makeInvalid();
    static StageID fromValue(Type value);

    ~StageID();

    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(StageID other) const { return value_ == other.value_; }
    bool operator!=(StageID other) const { return value_ != other.value_; }
    bool operator<(StageID other) const { return value_ < other.value_; }

private:
    StageID(Type value);

private:
    Type value_;
};

}
