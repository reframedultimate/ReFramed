#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterFlags
{
public:
    typedef uint8_t Type;

    static FighterFlags fromValue(Type value);
    static FighterFlags fromFlags(bool attackConnected, bool facingDirection, bool opponentInHitlag);

    ~FighterFlags();

    Type value() const { return value_; }
    bool attackConnected() const { return !!(value_ & 1); }
    bool facingLeft() const { return !!(value_ & 2); }
    bool opponentInHitlag() const { return !!(value_ & 4); }

    bool operator==(FighterFlags other) const { return value_ == other.value_; }
    bool operator!=(FighterFlags other) const { return value_ != other.value_; }
    bool operator<(FighterFlags other) const { return value_ < other.value_; }

private:
    FighterFlags(Type value);

private:
    Type value_;
};

}
