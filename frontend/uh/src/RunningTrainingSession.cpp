#include "uh/RunningTrainingSession.hpp"

namespace uh {

// ----------------------------------------------------------------------------
RunningTrainingSession::RunningTrainingSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags)
    : Session(std::move(mapping), stageID, std::move(playerFighterIDs), std::move(playerTags))
    , RunningSession()
    , TrainingSession()
{
}

}
