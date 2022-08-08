#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API GameNumber
{
public:
    typedef uint16_t Type;

    static GameNumber fromValue(Type value);

    ~GameNumber();

    Type value() const { return value_; }
    bool operator==(GameNumber rhs) { return value_ == rhs.value_; }
    bool operator!=(GameNumber rhs) { return value_ != rhs.value_; }
    GameNumber& operator+=(int value) { value_ += value; return *this; }
    GameNumber& operator-=(int value) { value_ -= value; return *this; }

private:
    GameNumber(Type value);

private:
    Type value_;
};

inline GameNumber operator+(GameNumber lhs, int value) { lhs += value; return lhs; }
inline GameNumber operator-(GameNumber lhs, int value) { lhs -= value; return lhs; }

}
