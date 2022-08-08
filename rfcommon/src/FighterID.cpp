#include "rfcommon/FighterID.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterID FighterID::fromValue(Type value)
{
    return FighterID(value);
}

// ----------------------------------------------------------------------------
FighterID FighterID::makeInvalid()
{
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
