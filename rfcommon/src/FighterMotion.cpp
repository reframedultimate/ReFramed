#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromValue(Type value)
{
    NOPROFILE();

    return FighterMotion(value);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromParts(uint8_t upper, uint32_t lower)
{
    NOPROFILE();

    return FighterMotion((static_cast<Type>(upper) << 32) | lower);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::makeInvalid()
{
    NOPROFILE();

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
