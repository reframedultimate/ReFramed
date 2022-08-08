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
FighterStatus::FighterStatus(Type value)
    : value_(value)
{}

}
