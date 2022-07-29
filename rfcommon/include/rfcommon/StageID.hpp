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

    Type value() const;
    bool isValid() const;

    bool operator==(StageID other) const;
    bool operator!=(StageID other) const;
    bool operator<(StageID other) const;

private:
    StageID();
    StageID(Type value);

private:
    Type value_;
};

}
