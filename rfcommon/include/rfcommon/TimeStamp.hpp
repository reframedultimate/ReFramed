#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/DeltaTime.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API TimeStamp
{
public:
    typedef uint64_t Type;

    static TimeStamp fromMillisSinceEpoch(Type value);
    static TimeStamp makeInvalid();

    ~TimeStamp();

    Type millisSinceEpoch() const;

    bool isValid() const;

    bool operator==(TimeStamp rhs) const;
    bool operator!=(TimeStamp rhs) const;
    bool operator< (TimeStamp rhs) const;
    bool operator<=(TimeStamp rhs) const;
    bool operator> (TimeStamp rhs) const;
    bool operator>=(TimeStamp rhs) const;
    TimeStamp& operator+=(DeltaTime rhs);
    TimeStamp& operator-=(DeltaTime rhs);

private:
    TimeStamp();
    TimeStamp(Type millisSinceEpoch);

private:
    Type millisSinceEpoch_;
};

inline TimeStamp operator+(TimeStamp lhs, DeltaTime rhs) { lhs += rhs; return lhs; }
inline TimeStamp operator-(TimeStamp lhs, DeltaTime rhs) { lhs -= rhs; return lhs; }
inline DeltaTime operator-(TimeStamp lhs, TimeStamp rhs) { return DeltaTime::fromMillis(lhs.millisSinceEpoch() - rhs.millisSinceEpoch()); }

}
