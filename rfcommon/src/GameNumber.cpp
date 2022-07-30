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
bool GameNumber::operator==(const GameNumber& rhs)
{
    return value_ == rhs.value_;
}

// ----------------------------------------------------------------------------
bool GameNumber::operator!=(const GameNumber& rhs)
{
    return value_ != rhs.value_;
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
