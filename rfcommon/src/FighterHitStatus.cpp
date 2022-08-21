#include "rfcommon/FighterHitStatus.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterHitStatus FighterHitStatus::fromValue(Type value)
{
    NOPROFILE();

    return FighterHitStatus(value);
}

// ----------------------------------------------------------------------------
FighterHitStatus FighterHitStatus::makeInvalid()
{
    NOPROFILE();

    return FighterHitStatus(Type(-1));
}

// ----------------------------------------------------------------------------
FighterHitStatus::~FighterHitStatus()
{}

// ----------------------------------------------------------------------------
FighterHitStatus::FighterHitStatus(Type value)
    : value_(value)
{}

}
