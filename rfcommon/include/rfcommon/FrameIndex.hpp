#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API FrameIndex
{
public:
    typedef uint32_t Type;

    static FrameIndex fromValue(Type value);
    static FrameIndex fromSeconds(double seconds);
    static FrameIndex makeInvalid();

    ~FrameIndex();

    bool isValid() const { return value_ != Type(-1); }
    Type index() const { return value_; }
    double secondsPassed() const { return static_cast<double>(value_) / 60.0; }
    uint64_t millisPassed() const { return static_cast<uint64_t>(value_) * 1000 / 60; }

    bool operator==(FrameIndex other) const { return value_ == other.value_; }
    bool operator!=(FrameIndex other) const { return value_ != other.value_; }
    bool operator< (FrameIndex other) const { return value_ < other.value_; }
    bool operator> (FrameIndex other) const { return value_ > other.value_; }
    bool operator<=(FrameIndex other) const { return value_ <= other.value_; }
    bool operator>=(FrameIndex other) const { return value_ >= other.value_; }

    FrameIndex& operator+=(FrameIndex rhs) { value_ += rhs.value_; return *this; }
    FrameIndex& operator+=(int rhs) { value_ += rhs; return *this; }
    FrameIndex& operator-=(FrameIndex rhs) { value_ -= rhs.value_; return *this; }
    FrameIndex& operator-=(int rhs) { value_ -= rhs; return *this; }

private:
    FrameIndex(Type value);

private:
    Type value_;
};

inline FrameIndex operator+(FrameIndex lhs, FrameIndex rhs) { lhs += rhs; return lhs; }
inline FrameIndex operator+(FrameIndex lhs, int rhs) { lhs += rhs; return lhs; }
inline FrameIndex operator-(FrameIndex lhs, FrameIndex rhs) { lhs -= rhs; return lhs; }
inline FrameIndex operator-(FrameIndex lhs, int rhs) { lhs -= rhs; return lhs; }

}
