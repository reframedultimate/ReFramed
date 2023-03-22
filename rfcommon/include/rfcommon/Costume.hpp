#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API Costume
{
public:
    typedef uint8_t Type;

    static Costume fromValue(Type value);
    static Costume makeDefault();

    ~Costume();

    Type value() const { return value_; }
    Type slot() const { return value_; }

private:
    Costume(Type value);

private:
    Type value_;
};

}
