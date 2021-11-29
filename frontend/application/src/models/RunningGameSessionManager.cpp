#include "application/Util.hpp"
#include "application/listeners/RunningGameSessionManagerListener.hpp"
#include "application/models/RunningGameSessionManager.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "uh/tcp_socket.h"
#include <QDateTime>

namespace uhapp {

// ----------------------------------------------------------------------------
RunningGameSessionManager::RunningGameSessionManager(Protocol* protocol, ReplayManager* manager, QObject *parent)
    : QObject(parent)
    , protocol_(protocol)
    , savedSessionManager_(manager)
    , format_(uh::SetFormat::FRIENDLIES)
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
void RunningGameSessionManager::setFormat(const uh::SetFormat& format)
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

        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP1NameChanged,
                            QString(activeSession_->playerName(0).cStr()));
    }
    else
    {
        p1Name_ = name;
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP1NameChanged, name);
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

        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP2NameChanged,
                            QString(activeSession_->playerName(1).cStr()));
    }
    else
    {
        p2Name_ = name;
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP2NameChanged, name);
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::setGameNumber(uh::GameNumber number)
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
bool RunningGameSessionManager::shouldStartNewSet(const uh::RunningGameSession* recording)
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
        case uh::SetFormat::BO3: {
            if (win[0] >= 2 || win[1] >= 2)
                return true;
        } break;

        case uh::SetFormat::BO5: {
            if (win[0] >= 3 || win[1] >= 3)
                return true;
        } break;

        case uh::SetFormat::BO7: {
            if (win[0] >= 4 || win[1] >= 4)
                return true;
        } break;

        case uh::SetFormat::FT5: {
            if (win[0] >= 5 || win[1] >= 5)
                return true;
        } break;

        case uh::SetFormat::FT10: {
            if (win[0] >= 10 || win[1] >= 10)
                return true;
        } break;

        case uh::SetFormat::FRIENDLIES:
        case uh::SetFormat::PRACTICE:
        case uh::SetFormat::OTHER:
            break;
    }

    return false;
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::findUniqueGameAndSetNumbers(uh::RunningGameSession* recording)
{
    const QDir& dir = savedSessionManager_->defaultRecordingSourceDirectory();
    while (QFileInfo::exists(dir.absoluteFilePath(composeFileName(recording))))
    {
        switch (format_.type())
        {
            case uh::SetFormat::FRIENDLIES:
            case uh::SetFormat::PRACTICE:
            case uh::SetFormat::OTHER:
                recording->setGameNumber(recording->gameNumber() + 1);
                break;

            default:
                recording->setSetNumber(recording->setNumber() + 1);
                break;
        }
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolAttemptConnectToServer()
{

}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolFailedToConnectToServer()
{

}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolConnectedToServer()
{

}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolDisconnectedFromServer()
{

}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolTrainingStarted(uh::RunningTrainingSession* session)
{

}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolTrainingEnded(uh::RunningTrainingSession* session)
{

}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolMatchStarted(uh::RunningGameSession* session)
{
    // first off, copy the data we've stored from the UI into the new recording
    // so comparing previous recordings is consistent
    session->setFormat(format_);
    session->setGameNumber(gameNumber_);
    session->setSetNumber(setNumber_);
    if (session->playerCount() == 2)
    {
        if (p1Name_.length() > 0)
            session->setPlayerName(0, p1Name_.toStdString().c_str());
        if (p2Name_.length() > 0)
            session->setPlayerName(1, p2Name_.toStdString().c_str());
    }

    if (shouldStartNewSet(session))
    {
        session->setGameNumber(1);
        session->setSetNumber(1);
        pastSessions_.clear();
    }
    else
    {
        // Go to the next game in the set
        session->setGameNumber(session->gameNumber() + 1);
    }

    // Modify game/set numbers until we have a unique filename
    findUniqueGameAndSetNumbers(session);

    session->dispatcher.addListener(this);
    activeSession_ = session;
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerRecordingStarted, session);
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerSetNumberChanged, session->setNumber());
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, session->gameNumber());
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerFormatChanged, session->format());
    if (session->playerCount() == 2)
    {
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP1NameChanged,
                            QString(session->playerName(0).cStr()));
        dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP2NameChanged,
                            QString(session->playerName(1).cStr()));
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onProtocolMatchEnded(uh::RunningGameSession* session)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerRecordingEnded, session);

    // Save recording
    QFileInfo fileInfo(
        savedSessionManager_->defaultRecordingSourceDirectory(),
        composeFileName(session)
    );
    if (session->save(fileInfo.absoluteFilePath().toStdString().c_str()))
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
    format_ = session->format();
    gameNumber_ = session->gameNumber();
    setNumber_ = session->setNumber();
    if (session->playerCount() == 2)
    {
        p1Name_ = session->playerName(0).cStr();
        p2Name_ = session->playerName(1).cStr();
    }

    session->dispatcher.removeListener(this);
    pastSessions_.push_back(session);
    activeSession_.reset();
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
void RunningGameSessionManager::onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name)
{
    if (activeSession_->playerCount() == 2)
    {
        if (player == 0)
            dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP1NameChanged, QString(name.cStr()));
        else
            dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerP2NameChanged, QString(name.cStr()));
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionSetNumberChanged(uh::SetNumber number)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionGameNumberChanged(uh::GameNumber number)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionFormatChanged(const uh::SetFormat& format)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningSessionNewUniquePlayerState(int player, const uh::PlayerState& state)
{
    (void)player;
    (void)state;
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningSessionNewPlayerState(int player, const uh::PlayerState& state)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerPlayerStateAdded, player, state);
}

// ----------------------------------------------------------------------------
void RunningGameSessionManager::onRunningGameSessionWinnerChanged(int winner)
{
    dispatcher.dispatch(&RunningGameSessionManagerListener::onRunningGameSessionManagerWinnerChanged, winner);
}

}
