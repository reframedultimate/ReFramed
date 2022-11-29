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
    static TimeStamp fromSecondsSinceEpoch(Type value);
    static TimeStamp makeInvalid();

    ~TimeStamp();

    Type millisSinceEpoch() const { return millisSinceEpoch_; }

    bool isValid() const { return millisSinceEpoch_ != 0; }

    bool operator==(TimeStamp rhs) const { return millisSinceEpoch_ == rhs.millisSinceEpoch_; }
    bool operator!=(TimeStamp rhs) const { return millisSinceEpoch_ != rhs.millisSinceEpoch_; }
    bool operator< (TimeStamp rhs) const { return millisSinceEpoch_ < rhs.millisSinceEpoch_; }
    bool operator<=(TimeStamp rhs) const { return millisSinceEpoch_ <= rhs.millisSinceEpoch_; }
    bool operator> (TimeStamp rhs) const { return millisSinceEpoch_ > rhs.millisSinceEpoch_; }
    bool operator>=(TimeStamp rhs) const { return millisSinceEpoch_ >= rhs.millisSinceEpoch_; }
    TimeStamp& operator+=(DeltaTime rhs) { millisSinceEpoch_ += rhs.millis(); return *this; }
    TimeStamp& operator-=(DeltaTime rhs) { millisSinceEpoch_ -= rhs.millis(); return *this; }

private:
    TimeStamp(Type millisSinceEpoch);

private:
    Type millisSinceEpoch_;
};

inline TimeStamp operator+(TimeStamp lhs, DeltaTime rhs) { lhs += rhs; return lhs; }
inline TimeStamp operator-(TimeStamp lhs, DeltaTime rhs) { lhs -= rhs; return lhs; }
inline DeltaTime operator-(TimeStamp lhs, TimeStamp rhs) { return DeltaTime::fromMillis(lhs.millisSinceEpoch() - rhs.millisSinceEpoch()); }

}
