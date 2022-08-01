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
uint8_t FighterMotion::upper() const
{
    return (value_ >> 32) & 0xFF;
}

// ----------------------------------------------------------------------------
uint32_t FighterMotion::lower() const
{
    return value_ & 0xFFFFFFFF;
}

// ----------------------------------------------------------------------------
FighterMotion::Type FighterMotion::value() const
{
    return value_;
}

// ----------------------------------------------------------------------------
bool FighterMotion::isValid() const
{
    return value_ != 0;
}

// ----------------------------------------------------------------------------
bool FighterMotion::operator==(FighterMotion other) const
{
    return value_ == other.value_;
}

// ----------------------------------------------------------------------------
bool FighterMotion::operator!=(FighterMotion other) const
{
    return value_ != other.value_;
}

// ----------------------------------------------------------------------------
bool FighterMotion::operator<(FighterMotion other) const
{
    return value_ < other.value_;
}

// ----------------------------------------------------------------------------
FighterMotion::FighterMotion()
{}

// ----------------------------------------------------------------------------
FighterMotion::FighterMotion(Type value)
    : value_(value)
{}

}
