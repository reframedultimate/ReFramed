#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API SessionNumber
{
public:
    typedef uint16_t Type;

    static SessionNumber fromValue(Type value);

    ~SessionNumber();

    Type value() const { return value_; }
    bool operator==(SessionNumber rhs) { return value_ == rhs.value_; }
    bool operator!=(SessionNumber rhs) { return value_ != rhs.value_; }
    SessionNumber& operator+=(int value) { value_ += value; return *this; }
    SessionNumber& operator-=(int value) { value_ -= value; return *this; }

private:
    SessionNumber(Type value);

private:
    Type value_;
};

inline SessionNumber operator+(SessionNumber lhs, int value) { lhs += value; return lhs; }
inline SessionNumber operator-(SessionNumber lhs, int value) { lhs -= value; return lhs; }

}
