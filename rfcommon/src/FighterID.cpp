#include "rfcommon/FighterID.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterID FighterID::fromValue(Type value) 
{ 
    return FighterID(value);
}

// ----------------------------------------------------------------------------
FighterID FighterID::makeInvalid()
{ 
    return FighterID(Type(-1));
}

// ----------------------------------------------------------------------------
FighterID::~FighterID()
{}

// ----------------------------------------------------------------------------
FighterID::Type FighterID::value() const 
{ 
    return value_; 
}

// ----------------------------------------------------------------------------
bool FighterID::isValid() const 
{ 
    return value_ != Type(-1);
}

// ----------------------------------------------------------------------------
bool FighterID::operator==(FighterID other) const 
{ 
    return value_ == other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterID::operator!=(FighterID other) const
{ 
    return value_ != other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterID::operator<(FighterID other) const
{
    return value_ < other.value_; 
}

// ----------------------------------------------------------------------------
FighterID::FighterID() 
    : value_(Type(-1)) 
{}

// ----------------------------------------------------------------------------
FighterID::FighterID(Type value)
    : value_(value) 
{}

}
