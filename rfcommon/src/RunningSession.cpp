#include "rfcommon/RunningSession.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/PlayerState.hpp"

namespace rfcommon {

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
    , frameUniqueBits_(0)
{
}

// ----------------------------------------------------------------------------
void RunningSession::addPlayerState(int playerIdx, PlayerState&& state)
{
    frameUniqueBits_ <<= 1;

    // Only add a new state if the previous one was different
    if (playerStates_[playerIdx].count() == 0 || playerStates_[playerIdx].back() != state)
    {
        playerStates_[playerIdx].push(state);
        dispatcher.dispatch(&SessionListener::onRunningSessionNewUniquePlayerState, playerIdx, state);

        frameUniqueBits_ |= 1;
    }

    // The UI still cares about every frame
    dispatcher.dispatch(&SessionListener::onRunningSessionNewPlayerState, playerIdx, state);

    // Reached end of frame
    if (playerIdx == playerCount() - 1)
    {
        rfcommon::SmallVector<PlayerState, 8> states;
        for (int i = 0; i != playerCount(); ++i)
        {
            if (playerStateCount(i) < 1)
                return;

            states.push(playerStateAt(i, playerStateCount(i) - 1));
        }

        if (frameUniqueBits_)
            dispatcher.dispatch(&SessionListener::onRunningSessionNewUniqueFrame, states);
        frameUniqueBits_ = 0;

        dispatcher.dispatch(&SessionListener::onRunningSessionNewFrame, states);
    }
}

}
