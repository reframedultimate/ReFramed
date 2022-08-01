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

    Type value() const;
    bool attackConnected() const;
    bool facingLeft() const;
    bool opponentInHitlag() const;

    bool operator==(FighterFlags other) const;
    bool operator!=(FighterFlags other) const;
    bool operator<(FighterFlags other) const;

private:
    FighterFlags();
    FighterFlags(Type value);

private:
    Type value_;
};

}
