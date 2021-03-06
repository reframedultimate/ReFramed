#include "rfcommon/RunningTrainingSession.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/SessionListener.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
RunningTrainingSession::RunningTrainingSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags)
    : Session(std::move(mapping), stageID, std::move(playerFighterIDs), std::move(playerTags))
    , RunningSession()
    , TrainingSession()
    , frameCounter_(0)
{
}

// ----------------------------------------------------------------------------
void RunningTrainingSession::addPlayerState(int playerIdx, PlayerState&& state)
{
    if (playerIdx == 0)
        frameCounter_++;

    RunningSession::addPlayerState(playerIdx,
        state.copyWithModifiedFrame(frameCounter_));
}

}
