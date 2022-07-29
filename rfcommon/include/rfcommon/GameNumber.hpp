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

    Type value() const;
    GameNumber& operator+=(int value);
    GameNumber& operator-=(int value);

private:
    GameNumber(Type value);
    Type value_;
};

inline GameNumber operator+(GameNumber lhs, int value) { lhs += value; return lhs; }
inline GameNumber operator-(GameNumber lhs, int value) { lhs -= value; return lhs; }

}
