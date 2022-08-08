#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API SetNumber
{
public:
    typedef uint16_t Type;

    static SetNumber fromValue(Type value);

    ~SetNumber();

    Type value() const { return value_; }

    bool operator==(SetNumber rhs) const { return value_ == rhs.value_; }
    bool operator!=(SetNumber rhs) const { return value_ != rhs.value_; }
    SetNumber& operator+=(int value) { value_ += value; return *this; }
    SetNumber& operator-=(int value) { value_ -= value; return *this; }

private:
    SetNumber(Type value);

private:
    Type value_;
};

inline SetNumber operator+(SetNumber lhs, int value) { lhs += value; return lhs; }
inline SetNumber operator-(SetNumber lhs, int value) { lhs -= value; return lhs; }

}
