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
bool SetNumber::operator==(const SetNumber& rhs) const
{
    return value_ == rhs.value_;
}

// ----------------------------------------------------------------------------
bool SetNumber::operator!=(const SetNumber& rhs) const
{
    return value_ != rhs.value_;
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
