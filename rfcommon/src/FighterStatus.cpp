#include "rfcommon/FighterStatus.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterStatus FighterStatus::fromValue(Type value)
{
    return FighterStatus(value);
}

// ----------------------------------------------------------------------------
FighterStatus FighterStatus::makeInvalid()
{
    return FighterStatus(Type(-1));
}

// ----------------------------------------------------------------------------
FighterStatus::Type FighterStatus::value() const
{
    assert(isValid());
    return value_;
}

// ----------------------------------------------------------------------------
bool FighterStatus::isValid() const
{
    return value_ != Type(-1);
}
// ----------------------------------------------------------------------------

bool FighterStatus::operator==(FighterStatus other) const
{
    return value_ == other.value_;
}

// ----------------------------------------------------------------------------
bool FighterStatus::operator!=(FighterStatus other) const
{
    return value_ != other.value_;
}

// ----------------------------------------------------------------------------
bool FighterStatus::operator<(FighterStatus other) const
{
    return value_ < other.value_;
}

// ----------------------------------------------------------------------------
FighterStatus::FighterStatus()
{}

// ----------------------------------------------------------------------------
FighterStatus::FighterStatus(Type value)
    : value_(value)
{}

}
