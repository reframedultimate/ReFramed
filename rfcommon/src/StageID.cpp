#include "rfcommon/StageID.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
StageID StageID::makeInvalid()
{
    return StageID(Type(-1));
}

// ----------------------------------------------------------------------------
StageID StageID::fromValue(Type value)
{
    return StageID(value);
}

// ----------------------------------------------------------------------------
StageID::~StageID()
{}

// ----------------------------------------------------------------------------
StageID::StageID(Type value)
    : value_(value)
{}

}
