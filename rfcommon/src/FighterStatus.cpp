#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/Profiler.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterStatus FighterStatus::fromValue(Type value)
{
    NOPROFILE();

    return FighterStatus(value);
}

// ----------------------------------------------------------------------------
FighterStatus FighterStatus::makeInvalid()
{
    NOPROFILE();

    return FighterStatus(Type(-1));
}

// ----------------------------------------------------------------------------
FighterStatus::FighterStatus(Type value)
    : value_(value)
{}

}
