#include "rfcommon/FighterFlags.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterFlags FighterFlags::fromValue(Type value)
{
    return FighterFlags(value);
}

// ----------------------------------------------------------------------------
FighterFlags FighterFlags::fromFlags(bool attackConnected, bool facingDirection, bool opponentInHitlag)
{
    return FighterFlags(
        (static_cast<Type>(attackConnected) << 0)
      | (static_cast<Type>(facingDirection) << 1)
      | (static_cast<Type>(opponentInHitlag) << 2));
}

// ----------------------------------------------------------------------------
FighterFlags::~FighterFlags()
{}

// ----------------------------------------------------------------------------
FighterFlags::FighterFlags(Type value)
    : value_(value)
{}


}
