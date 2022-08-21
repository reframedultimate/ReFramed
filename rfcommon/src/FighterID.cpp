#include "rfcommon/FighterID.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterID FighterID::fromValue(Type value)
{
    NOPROFILE();

    return FighterID(value);
}

// ----------------------------------------------------------------------------
FighterID FighterID::makeInvalid()
{
    NOPROFILE();

    return FighterID(Type(-1));
}

// ----------------------------------------------------------------------------
FighterID::~FighterID()
{}

// ----------------------------------------------------------------------------
FighterID::FighterID(Type value)
    : value_(value)
{}

}
