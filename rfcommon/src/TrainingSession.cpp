#include "rfcommon/TrainingSession.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/time.h"

namespace rfcommon {

// ----------------------------------------------------------------------------
TrainingSession::TrainingSession()
    : timeStarted_(time_milli_seconds_since_epoch())
{
}

// ----------------------------------------------------------------------------
FighterID TrainingSession::playerFighterID() const
{
    return Session::playerFighterID(0);
}

// ----------------------------------------------------------------------------
FighterID TrainingSession::cpuFighterID() const
{
    return Session::playerFighterID(1);
}

}
