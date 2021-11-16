#include "uh/RunningSession.hpp"
#include "uh/SessionListener.hpp"
#include "uh/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
RunningSession::RunningSession()
{
}

// ----------------------------------------------------------------------------
RunningSession::RunningSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags)
    : Session(std::move(mapping), stageID, std::move(playerFighterIDs), std::move(playerTags))
{
}

// ----------------------------------------------------------------------------
void RunningSession::addPlayerState(int playerIdx, PlayerState&& state)
{
    // Only add a new state if the previous one was different
    if (playerStates_[playerIdx].count() == 0 || playerStates_[playerIdx].back() != state)
    {
        playerStates_[playerIdx].push(state);
        dispatcher.dispatch(&SessionListener::onRunningSessionNewUniquePlayerState, playerIdx, state);
    }

    // The UI still cares about every frame
    dispatcher.dispatch(&SessionListener::onRunningSessionNewPlayerState, playerIdx, state);
}

}
