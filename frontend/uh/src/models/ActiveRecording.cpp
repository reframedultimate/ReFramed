#include "uh/listeners/RecordingListener.hpp"
#include "uh/models/ActiveRecording.hpp"
#include "uh/models/PlayerState.hpp"
#include <QFile>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>

namespace uh {

// ----------------------------------------------------------------------------
ActiveRecording::ActiveRecording(MappingInfo&& mapping,
                                 QVector<uint8_t>&& playerFighterIDs,
                                 QVector<QString>&& playerTags,
                                 uint16_t stageID)
    : Recording(
          std::move(mapping),
          std::move(playerFighterIDs),
          std::move(playerTags),
          stageID
      )
{
}

// ----------------------------------------------------------------------------
void ActiveRecording::setPlayerName(int index, const QString& name)
{
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
    if (playerStates_[index].count() == 0)
    {
        playerStates_[index].push_back(std::move(state));
        return;
    }

    // Check to see if the game has started yet
    if (playerStates_[index].count() == 1)
    {
        if (playerStates_[index].back().frame() == state.frame())
            return;  // has not started yet

        // The game has started. Adjust time started to be on the first frame
        // of gameplay
        timeStarted_ = QDateTime::currentDateTime();
        playerStates_[index].push_back(std::move(state));
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewUniquePlayerState, index, playerStates_[index].back());
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewPlayerState, index, playerStates_[index].back());
        return;
    }

    // Only add a new state if the previous one was different
    if (playerStates_[index].back() != state)
    {
        playerStates_[index].push_back(std::move(state));
        dispatcher.dispatch(&RecordingListener::onActiveRecordingNewUniquePlayerState, index, playerStates_[index].back());

        // Winner might have changed
        int winner = Recording::findWinner();
        if (winner_ != winner)
        {
            winner_ = winner;
            dispatcher.dispatch(&RecordingListener::onRecordingWinnerChanged, winner_);
        }
    }

    // The UI still cares about every frame
    dispatcher.dispatch(&RecordingListener::onActiveRecordingNewPlayerState, index, state);
}

}
