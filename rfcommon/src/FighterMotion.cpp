#include "rfcommon/FighterMotion.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromValue(Type value)
{
    return FighterMotion(value);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromParts(uint8_t upper, uint32_t lower)
{
    return FighterMotion((static_cast<Type>(upper) << 32) | lower);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::makeInvalid()
{
    return FighterMotion(0);
}

// ----------------------------------------------------------------------------
FighterMotion::~FighterMotion()
{}

// ----------------------------------------------------------------------------
FighterMotion::FighterMotion(Type value)
    : value_(value)
{}

}
