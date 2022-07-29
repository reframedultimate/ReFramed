#include "rfcommon/FighterHitStatus.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterHitStatus FighterHitStatus::fromValue(Type value)
{
    return FighterHitStatus(value);
}

// ----------------------------------------------------------------------------
FighterHitStatus FighterHitStatus::makeInvalid()
{
    return FighterHitStatus(Type(-1));
}

// ----------------------------------------------------------------------------
FighterHitStatus::~FighterHitStatus()
{}

// ----------------------------------------------------------------------------
FighterHitStatus::Type FighterHitStatus::value() const 
{ 
    return value_; 
}

// ----------------------------------------------------------------------------
bool FighterHitStatus::isValid() const
{
    return value_ != Type(-1);
}

// ----------------------------------------------------------------------------
bool FighterHitStatus::operator==(FighterHitStatus other) const 
{ 
    return value_ == other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterHitStatus::operator<(FighterHitStatus other) const
{ 
    return value_ < other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterHitStatus::operator!=(FighterHitStatus other) const 
{ 
    return value_ != other.value_; 
}

// ----------------------------------------------------------------------------
FighterHitStatus::FighterHitStatus() 
{}

// ----------------------------------------------------------------------------
FighterHitStatus::FighterHitStatus(Type value) 
    : value_(value) 
{}

}
