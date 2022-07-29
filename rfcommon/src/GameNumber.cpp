#include "rfcommon/GameNumber.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameNumber GameNumber::fromValue(Type value)
{
    return GameNumber(value);
}

// ----------------------------------------------------------------------------
GameNumber::~GameNumber()
{}

// ----------------------------------------------------------------------------
GameNumber::Type GameNumber::value() const 
{
    return value_;
}

// ----------------------------------------------------------------------------
GameNumber& GameNumber::operator+=(int value) 
{ 
    value_ += value; 
    return *this; 
}

// ----------------------------------------------------------------------------
GameNumber& GameNumber::operator-=(int value)
{ 
    value_ -= value;
    return *this;
}

// ----------------------------------------------------------------------------
GameNumber::GameNumber(Type value)
    : value_(value) 
{}

}
