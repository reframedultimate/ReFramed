#include "uh/RunningTrainingSession.hpp"
#include "uh/PlayerState.hpp"
#include "uh/SessionListener.hpp"

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

// ----------------------------------------------------------------------------
void RunningTrainingSession::resetTraining()
{
    dispatcher.dispatch(&SessionListener::onRunningTrainingSessionTrainingReset);
}

}
