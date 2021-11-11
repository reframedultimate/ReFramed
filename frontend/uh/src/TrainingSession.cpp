#include "uh/TrainingSession.hpp"
#include "uh/time.h"

namespace uh {

// ----------------------------------------------------------------------------
TrainingSession::TrainingSession(
        MappingInfo&& mapping,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags,
        StageID stageID)
    : Session(std::move(mapping), std::move(playerFighterIDs), std::move(playerTags), stageID)
    , timeStarted_(time_milli_seconds_since_epoch())
{
}

}
