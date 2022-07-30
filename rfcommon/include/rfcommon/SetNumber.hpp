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

    Type value() const;

    bool operator==(const SetNumber& rhs) const;
    bool operator!=(const SetNumber& rhs) const;
    SetNumber& operator+=(int value);
    SetNumber& operator-=(int value);

private:
    SetNumber(Type value);

private:
    Type value_;
};

inline SetNumber operator+(SetNumber lhs, int value) { lhs += value; return lhs; }
inline SetNumber operator-(SetNumber lhs, int value) { lhs -= value; return lhs; }

}
