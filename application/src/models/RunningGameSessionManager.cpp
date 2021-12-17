#include "application/Util.hpp"
#include "application/listeners/RunningGameSessionManagerListener.hpp"
#include "application/models/RunningGameSessionManager.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "rfcommon/tcp_socket.h"
#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
RunningGameSessionManager::RunningGameSessionManager(Protocol* protocol, ReplayManager* manager, QObject *parent)
    : QObject(parent)
    , protocol_(protocol)
    , savedSessionManager_(manager)
    , format_(rfcommon::SetFormat::FRIENDLIES)
{
    protocol_->dispatcher.addListener(this);
    savedSessionManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
RunningGameSessionManager::~RunningGameSessionManager()
{
    savedSessionManager_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);

    if (activeSession_)
        activeSession_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::setFormat(const rfcommon::SetFormat& format)
{
    if (activeSession_)
    {
        activeSession_->setFormat(format);
    }
    else
    {
        format_ = format;
    }

    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::setP1Name(const QString& name)
{
    if (activeSession_ && activeSession_->playerCount() == 2)
    {
        if (name == "")
            activeSession_->setPlayerName(0, activeSession_->playerTag(0));
        else
            activeSession_->setPlayerName(0, name.toStdString().c_str());

        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged,
                            0, activeSession_->playerName(0));
    }
    else
    {
        p1Name_ = name;
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged, 0, rfcommon::SmallString<15>(name.toStdString().c_str()));
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::setP2Name(const QString& name)
{
    if (activeSession_ && activeSession_->playerCount() == 2)
    {
        if (name == "")
            activeSession_->setPlayerName(1, activeSession_->playerTag(1));
        else
            activeSession_->setPlayerName(1, name.toStdString().c_str());

        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged,
                            1, activeSession_->playerName(1));
    }
    else
    {
        p2Name_ = name;
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged, 1, rfcommon::SmallString<15>(name.toStdString().c_str()));
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::setGameNumber(rfcommon::GameNumber number)
{
    if (activeSession_)
    {
        activeSession_->setGameNumber(number);
        findUniqueGameAndSetNumbers(activeSession_);

        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerSetNumberChanged, activeSession_->setNumber());
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, activeSession_->gameNumber());
    }
    else
    {
        gameNumber_ = number;
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, number);
    }
}

// ----------------------------------------------------------------------------
bool RunningGameSessionManager::isSessionRunning() const
{
    return protocol_->isConnected();
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::findUniqueGameAndSetNumbers(rfcommon::RunningGameSession* recording)
{
    const QDir& dir = savedSessionManager_->defaultRecordingSourceDirectory();
    while (QFileInfo::exists(dir.absoluteFilePath(composeFileName(recording))))
    {
        switch (format_.type())
        {
            case rfcommon::SetFormat::FRIENDLIES:
            case rfcommon::SetFormat::PRACTICE:
            case rfcommon::SetFormat::OTHER:
                recording->setGameNumber(recording->gameNumber() + 1);
                break;

            default:
                recording->setSetNumber(recording->setNumber() + 1);
                break;
        }
    }
}

// ----------------------------------------------------------------------------
bool RunningGameSessionManager::shouldStartNewSet(const rfcommon::RunningGameSession* recording)
{
    // For any game that doesn't have exactly 2 players we don't care about sets
    if (recording->playerCount() != 2)
        return true;

    // No past recordings? -> new set
    if (pastSessions_.size() == 0)
        return true;
    const auto& prev = pastSessions_.back();

    // Player tags might have changed
    if (prev->playerTag(0) != recording->playerTag(0) ||
        prev->playerTag(1) != recording->playerTag(1))
    {
        return true;
    }

    // Player names might have changed
    if (prev->playerName(0) != recording->playerName(0) ||
        prev->playerName(1) != recording->playerName(1))
    {
        return true;
    }

    // Format might have changed
    if (prev->format().type() != recording->format().type())
        return true;

    // tally up wins for each player
    int win[2] = {0, 0};
    for (const auto& rec : pastSessions_)
        win[rec->winner()]++;

    switch (recording->format().type())
    {
        case rfcommon::SetFormat::BO3: {
            if (win[0] >= 2 || win[1] >= 2)
                return true;
        } break;

        case rfcommon::SetFormat::BO5: {
            if (win[0] >= 3 || win[1] >= 3)
                return true;
        } break;

        case rfcommon::SetFormat::BO7: {
            if (win[0] >= 4 || win[1] >= 4)
                return true;
        } break;

        case rfcommon::SetFormat::FT5: {
            if (win[0] >= 5 || win[1] >= 5)
                return true;
        } break;

        case rfcommon::SetFormat::FT10: {
            if (win[0] >= 10 || win[1] >= 10)
                return true;
        } break;

        case rfcommon::SetFormat::FRIENDLIES:
        case rfcommon::SetFormat::PRACTICE:
        case rfcommon::SetFormat::OTHER:
            break;
    }

    return false;
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerAttemptConnectToServer, ipAddress, port);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerFailedToConnectToServer, ipAddress, port);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerConnectedToServer, ipAddress, port);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolDisconnectedFromServer()
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerDisconnectedFromServer);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training)
{
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training)
{
}

void RunningGameSessionManager::onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldTraining, rfcommon::RunningTrainingSession* newTraining)
{
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training)
{
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    // first off, copy the data we've stored from the UI into the new recording
    // so comparing previous recordings is consistent
    match->setFormat(format_);
    match->setGameNumber(gameNumber_);
    match->setSetNumber(setNumber_);
    if (match->playerCount() == 2)
    {
        if (p1Name_.length() > 0)
            match->setPlayerName(0, p1Name_.toStdString().c_str());
        if (p2Name_.length() > 0)
            match->setPlayerName(1, p2Name_.toStdString().c_str());
    }

    if (shouldStartNewSet(match))
    {
        match->setGameNumber(1);
        match->setSetNumber(1);
        pastSessions_.clear();
    }
    else
    {
        // Go to the next game in the set
        match->setGameNumber(match->gameNumber() + 1);
    }

    // Modify game/set numbers until we have a unique filename
    findUniqueGameAndSetNumbers(match);

    match->dispatcher.addListener(this);
    activeSession_ = match;
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerMatchStarted, match);
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerSetNumberChanged, match->setNumber());
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, match->gameNumber());
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerFormatChanged, match->format());
    if (match->playerCount() == 2)
    {
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged,
                            0, match->playerName(0));
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged,
                            1, match->playerName(1));
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerMatchEnded, match);

    // Save recording
    QFileInfo fileInfo(
        savedSessionManager_->defaultRecordingSourceDirectory(),
        composeFileName(match)
    );
    if (match->save(fileInfo.absoluteFilePath().toStdString().c_str()))
    {
        // Add the new recording to the "All" recording group
        savedSessionManager_->allReplayGroup()->addFile(fileInfo.absoluteFilePath());
    }
    else
    {
        // TODO: Need to handle this somehow
    }

    // In between recordings (when players are in the menu) there is no active
    // recording, but it's still possible to edit the names/format/game number/etc
    // so copy the data out of the recording here so it can be edited, and when
    // a new recording starts again we copy the data into the recording.
    format_ = match->format();
    gameNumber_ = match->gameNumber();
    setNumber_ = match->setNumber();
    if (match->playerCount() == 2)
    {
        p1Name_ = match->playerName(0).cStr();
        p2Name_ = match->playerName(1).cStr();
    }

    match->dispatcher.removeListener(this);
    pastSessions_.push_back(match);
    activeSession_.drop();
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path)
{
    (void)path;
    if (activeSession_)
    {
        findUniqueGameAndSetNumbers(activeSession_);

        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerSetNumberChanged, activeSession_->setNumber());
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, activeSession_->gameNumber());
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name)
{
    if (activeSession_->playerCount() == 2)
    {
        if (player == 0)
            dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged, 0, name);
        else
            dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerNameChanged, 1, name);
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state)
{
    (void)player;
    (void)state;
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerNewPlayerState, player, state);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionWinnerChanged(int winner)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerWinnerChanged, winner);
}

}
