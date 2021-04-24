#include "uh/RecordingListener.hpp"
#include "uh/ActiveRecording.hpp"
#include "uh/PlayerState.hpp"
#include "uh/time.h"
#include <cassert>

namespace uh {

// ----------------------------------------------------------------------------
ActiveRecording::ActiveRecording(MappingInfo&& mapping,
                                 std::vector<FighterID>&& playerFighterIDs,
                                 std::vector<std::string>&& playerTags,
                                 StageID stageID)
    : Recording(
          std::move(mapping),
          std::move(playerFighterIDs),
          std::move(playerTags),
          stageID
      )
{
}

// ----------------------------------------------------------------------------
void ActiveRecording::setPlayerName(int index, const std::string& name)
{
    assert(name.length() > 0);
    playerNames_[index] = name;
    dispatcher.dispatch(&RecordingListener::onActiveRecordingPlayerNameChanged, index, name);
}

// ----------------------------------------------------------------------------
void ActiveRecording::setGameNumber(int number)
{
    gameNumber_ = number;
    dispatcher.dispatch(&RecordingListener::onActiveRecordingGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveRecording::setSetNumber(int number)
{
    setNumber_ = number;
    dispatcher.dispatch(&RecordingListener::onActiveRecordingSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveRecording::setFormat(const SetFormat& format)
{
    format_ = format;
    dispatcher.dispatch(&RecordingListener::onActiveRecordingFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveRecording::addPlayerState(int index, PlayerState&& state)
{
    // If this is the first state we receive just store it. Need at least 1
    // past state to determine if the game started yet
    if (playerStates_[index].size() == 0)
    {
        playerStates_[index].push_back(std::move(state));
        return;
    }

    // Check to see if the game has started yet
    if (playerStates_[index].size() == 1)
    {
        if (playerStates_[index][0].frame() == state.frame())
        {
            playerStates_[index][0] = std::move(state);
            return;  // has not started yet
        }

        // Update first frame timestamp
        timeStarted_ = playerStates_[index][0].timeStampMs();
        timeEnded_ = timeStarted_;

        playerStates_[index].push_back(std::move(state));
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewUniquePlayerState, index, playerStates_[index][0]);
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewPlayerState, index, playerStates_[index][0]);
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewUniquePlayerState, index, playerStates_[index][1]);
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewPlayerState, index, playerStates_[index][1]);
        return;
    }

    // Only add a new state if the previous one was different
    if (playerStates_[index].back() != state)
    {
        playerStates_[index].push_back(state);
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewUniquePlayerState, index, state);

        // Winner might have changed
        int winner = Recording::findWinner();
        if (winner_ != winner)
        {
            winner_ = winner;
            dispatcher.dispatch(&RecordingListener::onRecordingWinnerChanged, winner_);
        }

        // update end timestamp
        timeEnded_ = state.timeStampMs();
    }

    // The UI still cares about every frame
    dispatcher.dispatch(&RecordingListener::onActiveRecordingNewPlayerState, index, state);
}

}
