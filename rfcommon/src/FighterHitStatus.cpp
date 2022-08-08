#include "rfcommon/FighterHitStatus.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterHitStatus FighterHitStatus::fromValue(Type value)
{
    return FighterHitStatus(value);
}

// ----------------------------------------------------------------------------
FighterHitStatus FighterHitStatus::makeInvalid()
{
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
