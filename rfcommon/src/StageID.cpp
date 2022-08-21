#include "rfcommon/StageID.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
StageID StageID::makeInvalid()
{
    NOPROFILE();

    return StageID(Type(-1));
}

// ----------------------------------------------------------------------------
StageID StageID::fromValue(Type value)
{
    NOPROFILE();

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
