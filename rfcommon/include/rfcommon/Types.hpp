#pragma once

#include <cstdint>
#include <string>
#include <cassert>
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
class FighterID
{
public:
    typedef uint8_t Type;

    static FighterID makeInvalid() { return FighterID(Type(-1)); }

    FighterID(Type value) : value_(value) {}
    std::string valueToStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(FighterID other) const { return value_ == other.value_; }
    bool operator!=(FighterID other) const { return value_ != other.value_; }
    bool operator<(FighterID other) const { return value_ < other.value_; }

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterID fighterID) const {
            return rfcommon::Hasher<Type>()(fighterID.value());
        }
    };

private:
    friend class SmallVector<FighterID, 2>;
    FighterID() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FighterStatus
{
public:
    typedef uint16_t Type;

    static FighterStatus makeInvalid() { return FighterStatus(Type(-1)); }

    FighterStatus(Type value) : value_(value) {}
    std::string valueToStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(FighterStatus other) const { return value_ == other.value_; }
    bool operator!=(FighterStatus other) const { return value_ != other.value_; }
    bool operator<(FighterStatus other) const { return value_ < other.value_; }

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterStatus fighterStatus) const {
            return rfcommon::Hasher<Type>()(fighterStatus.value());
        }
    };

private:
    friend class FighterState;
    FighterStatus() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FighterHitStatus
{
public:
    typedef uint8_t Type;

    FighterHitStatus(Type value) : value_(value) {}
    std::string toStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }

    bool operator==(FighterHitStatus other) const { return value_ == other.value_; }
    bool operator<(FighterHitStatus other) const { return value_ < other.value_; }
    bool operator!=(FighterHitStatus other) const { return value_ != other.value_; }

private:
    friend class FighterState;
    friend class SmallVector<FighterHitStatus, 6>;
    FighterHitStatus() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FighterMotion
{
public:
    typedef uint64_t Type;

    static FighterMotion makeInvalid() { return FighterMotion(0); }

    FighterMotion(Type value) : value_(value) {}
    FighterMotion(uint8_t upper, uint32_t lower)
        : value_((static_cast<Type>(upper) << 32) | lower) {}

    uint8_t upper() const { return (value_ >> 32) & 0xFF; }
    uint32_t lower() const { return value_ & 0xFFFFFFFF; }
    Type value() const { return value_; }
    std::string valueToStdString() const { return std::to_string(value_); }
    bool isValid() const { return value_ != 0; }

    bool operator==(FighterMotion other) const { return value_ == other.value_; }
    bool operator!=(FighterMotion other) const { return value_ != other.value_; }
    bool operator<(FighterMotion other) const { return value_ < other.value_; }

    // The motion value is already a hash value, but it's a 40-bit hash
    // value. Our hashmap needs a 32-bit value.
    struct Hasher {
        typedef uint32_t HashType;
        HashType operator()(FighterMotion motion) const {
            return hash32_combine(motion.lower(), motion.upper());
        }
    };

private:
    friend class FighterState;
    FighterMotion() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FighterFlags
{
public:
    typedef uint8_t Type;

    FighterFlags(Type value) : value_(value) {}
    FighterFlags(bool attackConnected, bool facingDirection, bool opponentInHitlag)
        : value_(
              (static_cast<Type>(attackConnected) << 0)
            | (static_cast<Type>(facingDirection) << 1)
            | (static_cast<Type>(opponentInHitlag) << 2)
          )
    {}

    Type value() const { return value_; }
    bool attackConnected() const { return !!(value_ & 1); }
    bool facingDirection() const { return !!(value_ & 2); }
    bool opponentInHitlag() const { return !!(value_ & 4); }

    bool operator==(FighterFlags other) const { return value_ == other.value_; }
    bool operator!=(FighterFlags other) const { return value_ != other.value_; }
    bool operator<(FighterFlags other) const { return value_ < other.value_; }

private:
    friend class FighterState;
    FighterFlags() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FighterStocks
{
public:
    typedef uint8_t Type;

    FighterStocks(Type value) : value_(value) {}
    Type value() const { return value_; }

    bool operator==(FighterStocks other) const { return value_ == other.value_; }
    bool operator!=(FighterStocks other) const { return value_ != other.value_; }
    bool operator< (FighterStocks other) const { return value_ < other.value_; }
    bool operator<=(FighterStocks other) const { return value_ <= other.value_; }
    bool operator> (FighterStocks other) const { return value_ > other.value_; }
    bool operator>=(FighterStocks other) const { return value_ >= other.value_; }

private:
    friend class FighterState;
    FighterStocks() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class GameNumber
{
public:
    typedef uint16_t Type;

    GameNumber(Type value) : value_(value) {}
    Type value() const { return value_; }
    GameNumber& operator+=(int value) { value_ += value; return *this; }
    GameNumber& operator-=(int value) { value_ -= value; return *this; }

private:
    Type value_;
};

inline GameNumber operator+(GameNumber lhs, int value) { lhs += value; return lhs; }
inline GameNumber operator-(GameNumber lhs, int value) { lhs -= value; return lhs; }

// ----------------------------------------------------------------------------
class SetNumber
{
public:
    typedef uint16_t Type;

    SetNumber(Type value) : value_(value) {}
    Type value() const { return value_; }

    bool operator==(int value) const { return value_ == value; }
    SetNumber& operator+=(int value) { value_ += value; return *this; }
    SetNumber& operator-=(int value) { value_ -= value; return *this; }

private:
    Type value_;
};

inline SetNumber operator+(SetNumber lhs, int value) { lhs += value; return lhs; }
inline SetNumber operator-(SetNumber lhs, int value) { lhs -= value; return lhs; }

// ----------------------------------------------------------------------------
class StageID
{
public:
    typedef uint16_t Type;

    static StageID makeInvalid() { return StageID(Type(-1)); }

    StageID(Type value) : value_(value) {}
    std::string valueToStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(StageID other) const { return value_ == other.value_; }
    bool operator!=(StageID other) const { return value_ != other.value_; }
    bool operator<(StageID other) const { return value_ < other.value_; }

private:
    friend class SmallVector<StageID, 10>;
    StageID() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FrameNumber
{
public:
    typedef uint32_t Type;

    FrameNumber(Type value) : value_(value) {}
    Type value() const { return value_; }
    double secondsPassed() const { return static_cast<double>(value_) / 60.0; }

    bool operator==(FrameNumber other) const { return value_ == other.value_; }
    bool operator!=(FrameNumber other) const { return value_ != other.value_; }

private:
    friend class FighterState;
    FrameNumber() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class FramesLeft
{
public:
    typedef uint32_t Type;

    FramesLeft(Type value) : value_(value) {}
    Type value() const { return value_; }
    double secondsLeft() const { return static_cast<double>(value_) / 60.0; }

    bool operator==(FramesLeft rhs) const { return value_ == rhs.value_; }
    bool operator!=(FramesLeft rhs) const { return value_ != rhs.value_; }

private:
    friend class FighterState;
    FramesLeft() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class DeltaTime
{
public:
    typedef uint64_t Type;

    static DeltaTime fromMillis(Type value) { DeltaTime dt; dt.value_ = value; return dt; }

    DeltaTime(Type value) : value_(value) {}
    Type millis() const { return value_; }

private:
    DeltaTime() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class TimeStamp
{
public:
    typedef uint64_t Type;

    static TimeStamp fromMillisSinceEpoch(Type value) { TimeStamp ts; ts.millisSinceEpoch_ = value; return ts; }

    std::string valueToStdString() const { return std::to_string(millisSinceEpoch_); }
    Type millisSinceEpoch() const { return millisSinceEpoch_; }

    bool isValid() const { return millisSinceEpoch_ != 0; }

    bool operator==(TimeStamp rhs) const { return millisSinceEpoch_ == rhs.millisSinceEpoch_; }
    bool operator!=(TimeStamp rhs) const { return millisSinceEpoch_ != rhs.millisSinceEpoch_; }
    bool operator< (TimeStamp rhs) const { return millisSinceEpoch_ <  rhs.millisSinceEpoch_; }
    bool operator<=(TimeStamp rhs) const { return millisSinceEpoch_ <= rhs.millisSinceEpoch_; }
    bool operator> (TimeStamp rhs) const { return millisSinceEpoch_ >  rhs.millisSinceEpoch_; }
    bool operator>=(TimeStamp rhs) const { return millisSinceEpoch_ >= rhs.millisSinceEpoch_; }
    TimeStamp& operator+=(DeltaTime rhs) { millisSinceEpoch_ += rhs.millis(); return *this; }
    TimeStamp& operator-=(DeltaTime rhs) { millisSinceEpoch_ -= rhs.millis(); return *this; }

private:
    friend class FighterState;
    TimeStamp() {}

private:
    Type millisSinceEpoch_;
};

inline TimeStamp operator+(TimeStamp lhs, DeltaTime rhs) { lhs += rhs; return lhs; }
inline TimeStamp operator-(TimeStamp lhs, DeltaTime rhs) { lhs -= rhs; return lhs; }
inline DeltaTime operator-(TimeStamp lhs, TimeStamp rhs) { return DeltaTime::fromMillis(lhs.millisSinceEpoch() - rhs.millisSinceEpoch()); }

}
