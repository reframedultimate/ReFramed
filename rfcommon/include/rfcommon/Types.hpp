#pragma once

#include <cstdint>
#include <string>
#include <cassert>
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
class FighterID
{
public:
    typedef uint8_t Type;

    FighterID(Type value) : value_(value) {}
    std::string toStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }

    bool operator==(FighterID other) const { return value_ == other.value_; }
    bool operator!=(FighterID other) const { return value_ != other.value_; }
    bool operator<(FighterID other) const { return value_ < other.value_; }

private:
    friend class SmallVector<FighterID, 2>;
    FighterID() {}

private:
    Type value_;
};

struct FighterIDHasher {
    typedef HashMapHasher<FighterID::Type>::HashType HashType;
    HashType operator()(FighterID fighterID) const {
        return HashMapHasher<FighterID::Type>()(fighterID.value());
    }
};

// ----------------------------------------------------------------------------
class FighterStatus
{
public:
    typedef uint16_t Type;

    FighterStatus(Type value) : value_(value) {}
    std::string toStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }

    bool operator==(FighterStatus other) const { return value_ == other.value_; }
    bool operator!=(FighterStatus other) const { return value_ != other.value_; }
    bool operator<(FighterStatus other) const { return value_ < other.value_; }

private:
    friend class FighterFrame;
    FighterStatus() {}

private:
    Type value_;
};

struct FighterStatusHasher {
    typedef HashMapHasher<FighterStatus::Type>::HashType HashType;
    HashType operator()(FighterStatus fighterStatus) const {
        return HashMapHasher<FighterStatus::Type>()(fighterStatus.value());
    }
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
    friend class FighterFrame;
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

    FighterMotion(Type value) : value_(value) {}
    FighterMotion(uint8_t upper, uint32_t lower)
        : value_((static_cast<Type>(upper) << 32) | lower) {}

    uint8_t upper() const { return (value_ >> 32) & 0xFF; }
    uint32_t lower() const { return value_ & 0xFFFFFFFF; }
    Type value() const { return value_; }

    bool operator==(FighterMotion other) const { return value_ == other.value_; }
    bool operator!=(FighterMotion other) const { return value_ != other.value_; }
    bool operator<(FighterMotion other) const { return value_ < other.value_; }

private:
    friend class FighterFrame;
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
    FighterFlags(bool attackConnected, bool facingDirection)
        : value_(
              (static_cast<Type>(attackConnected) << 0)
            | (static_cast<Type>(facingDirection) << 1))
    {}

    Type value() const { return value_; }
    bool attackConnected() const { return !!(value_ & 0); }
    bool facingDirection() const { return !!(value_ & 1); }

    bool operator==(FighterFlags other) const { return value_ == other.value_; }
    bool operator!=(FighterFlags other) const { return value_ != other.value_; }
    bool operator<(FighterFlags other) const { return value_ < other.value_; }

private:
    friend class FighterFrame;
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
    friend class FighterFrame;
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

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class SetNumber
{
public:
    typedef uint16_t Type;

    SetNumber(Type value) : value_(value) {}
    Type value() const { return value_; }

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class StageID
{
public:
    typedef uint16_t Type;

    StageID(Type value) : value_(value) {}
    std::string toStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }

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
class Frame
{
public:
    typedef uint32_t Type;

    Frame(Type value) : value_(value) {}
    Type value() const { return value_; }

private:
    friend class FighterFrame;
    Frame() {}

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class DeltaTimeMS
{
public:
    typedef uint64_t Type;

    DeltaTimeMS(Type value) : value_(value) {}
    Type value() const { return value_; }

private:
    Type value_;
};

// ----------------------------------------------------------------------------
class TimeStampMS
{
public:
    typedef uint64_t Type;

    TimeStampMS(Type value) : value_(value) {}
    std::string toStdString() const { return std::to_string(value_); }
    Type value() const { return value_; }

    bool operator==(TimeStampMS rhs) const { return value_ == rhs.value_; }
    bool operator!=(TimeStampMS rhs) const { return value_ != rhs.value_; }
    bool operator< (TimeStampMS rhs) const { return value_ <  rhs.value_; }
    bool operator<=(TimeStampMS rhs) const { return value_ <= rhs.value_; }
    bool operator> (TimeStampMS rhs) const { return value_ >  rhs.value_; }
    bool operator>=(TimeStampMS rhs) const { return value_ >= rhs.value_; }
    TimeStampMS& operator+=(DeltaTimeMS rhs) { value_ += rhs.value(); return *this; }
    TimeStampMS& operator-=(DeltaTimeMS rhs) { value_ -= rhs.value(); return *this; }

private:
    friend class FighterFrame;
    TimeStampMS() {}

private:
    Type value_;
};

inline TimeStampMS operator+(TimeStampMS lhs, DeltaTimeMS rhs) { lhs += rhs; return lhs; }
inline TimeStampMS operator-(TimeStampMS lhs, DeltaTimeMS rhs) { lhs -= rhs; return lhs; }
inline DeltaTimeMS operator-(TimeStampMS lhs, TimeStampMS rhs) { return lhs.value() - rhs.value(); }

}
