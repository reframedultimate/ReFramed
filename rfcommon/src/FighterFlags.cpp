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
FighterFlags::Type FighterFlags::value() const
{
    return value_; 
}

// ----------------------------------------------------------------------------
bool FighterFlags::attackConnected() const 
{
    return !!(value_ & 1);
}

// ----------------------------------------------------------------------------
bool FighterFlags::facingDirection() const
{ 
    return !!(value_ & 2); 
}

// ----------------------------------------------------------------------------
bool FighterFlags::opponentInHitlag() const 
{ 
    return !!(value_ & 4);
}

// ----------------------------------------------------------------------------
bool FighterFlags::operator==(FighterFlags other) const
{ 
    return value_ == other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterFlags::operator!=(FighterFlags other) const 
{
    return value_ != other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterFlags::operator<(FighterFlags other) const 
{
    return value_ < other.value_;
}

// ----------------------------------------------------------------------------
FighterFlags::FighterFlags()
    : value_(0)
{}

// ----------------------------------------------------------------------------
FighterFlags::FighterFlags(Type value) 
    : value_(value)
{}


}
