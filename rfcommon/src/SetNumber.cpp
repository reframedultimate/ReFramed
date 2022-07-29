#include "rfcommon/SetNumber.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
SetNumber SetNumber::fromValue(Type value)
{
    return SetNumber(value);
}

// ----------------------------------------------------------------------------
SetNumber::~SetNumber()
{}

// ----------------------------------------------------------------------------
SetNumber::Type SetNumber::value() const
{ 
    return value_; 
}

// ----------------------------------------------------------------------------
bool SetNumber::operator==(int value) const
{ 
    return value_ == value; 
}

// ----------------------------------------------------------------------------
SetNumber& SetNumber::operator+=(int value) 
{ 
    value_ += value; 
    return *this; 
}

// ----------------------------------------------------------------------------
SetNumber& SetNumber::operator-=(int value) 
{ 
    value_ -= value; 
    return *this; 
}

// ----------------------------------------------------------------------------
SetNumber::SetNumber(Type value) 
    : value_(value) 
{}

}
