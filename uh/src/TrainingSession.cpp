#include "uh/TrainingSession.hpp"
#include "uh/PlayerState.hpp"
#include "uh/time.h"

namespace uh {

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
