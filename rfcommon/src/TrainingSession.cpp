#include "rfcommon/TrainingSession.hpp"
#include "rfcommon/time.h"

namespace rfcommon {

// ----------------------------------------------------------------------------
TrainingSession::TrainingSession()
{
}

// ----------------------------------------------------------------------------
FighterID TrainingSession::playerFighterID() const
{
    return Session::fighterID(0);
}

// ----------------------------------------------------------------------------
FighterID TrainingSession::cpuFighterID() const
{
    return Session::fighterID(1);
}

}
