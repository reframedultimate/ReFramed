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
    // The winner is the player with most stocks and least damage
    winner_ = 0;
    for (int i = 0; i != playerStates_.count(); ++i)
    {
        if (playerStates_[i].count() == 0 || playerStates_[winner_].count() == 0)
            continue;

        const auto& current = playerStates_[i].back();
        const auto& winner = playerStates_[winner_].back();

        if (current.stocks() > winner.stocks())
            winner_ = i;
        else if (current.stocks() == winner.stocks())
            if (current.damage() < winner.damage())
                winner_ = i;
    }

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
        dispatcher.dispatch(&RecordingListener::onActiveRecordingPlayerStateAdded, index, playerStates_[index].back());
        return;
    }

    // Only add a new state if the previous one was different
    if (playerStates_[index].back().status() != state.status())
    {
        playerStates_[index].push_back(std::move(state));
        dispatcher.dispatch(&RecordingListener::onActiveRecordingPlayerStateAdded, index, playerStates_[index].back());
    }
    else
    {
        // The UI still cares about every frame so dispatch event even if it wasn't added
        dispatcher.dispatch(&RecordingListener::onActiveRecordingPlayerStateAdded, index, state);
    }
}

}
