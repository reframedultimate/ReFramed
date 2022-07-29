#include "rfcommon/StageID.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
StageID StageID::makeInvalid()
{
    return StageID(Type(-1));
}

// ----------------------------------------------------------------------------
StageID StageID::fromValue(Type value)
{
    return StageID(value);
}

// ----------------------------------------------------------------------------
StageID::~StageID()
{}

// ----------------------------------------------------------------------------
StageID::Type StageID::value() const
{
    return value_; 
}

// ----------------------------------------------------------------------------
bool StageID::isValid() const 
{ 
    return value_ != Type(-1);
}

// ----------------------------------------------------------------------------
bool StageID::operator==(StageID other) const
{ 
    return value_ == other.value_; 
}

// ----------------------------------------------------------------------------
bool StageID::operator!=(StageID other) const
{
    return value_ != other.value_;
}

// ----------------------------------------------------------------------------
bool StageID::operator<(StageID other) const
{
    return value_ < other.value_;
}

// ----------------------------------------------------------------------------
StageID::StageID()
    : value_(Type(-1))
{}

// ----------------------------------------------------------------------------
StageID::StageID(Type value) 
    : value_(value) 
{}

}
