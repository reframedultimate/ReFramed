#include "application/Util.hpp"
#include "application/listeners/ActiveRecordingManagerListener.hpp"
#include "application/models/ActiveRecordingManager.hpp"
#include "application/models/RecordingManager.hpp"
#include "uh/tcp_socket.h"
#include <QDateTime>

namespace uhapp {

// ----------------------------------------------------------------------------
ActiveRecordingManager::ActiveRecordingManager(RecordingManager* recordingManager, QObject *parent)
    : QObject(parent)
    , recordingManager_(recordingManager)
    , format_(uh::SetFormat::FRIENDLIES)
{
    recordingManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveRecordingManager::~ActiveRecordingManager()
{
    recordingManager_->dispatcher.removeListener(this);

    if (activeRecording_)
        activeRecording_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setFormat(const uh::SetFormat& format)
{
    if (activeRecording_)
    {
        activeRecording_->setFormat(format);
    }
    else
    {
        format_ = format;
    }

    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setP1Name(const QString& name)
{
    if (activeRecording_ && activeRecording_->playerCount() == 2)
    {
        if (name == "")
            activeRecording_->setPlayerName(0, activeRecording_->playerTag(0));
        else
            activeRecording_->setPlayerName(0, name.toStdString().c_str());

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP1NameChanged,
                            QString(activeRecording_->playerName(0).cStr()));
    }
    else
    {
        p1Name_ = name;
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP1NameChanged, name);
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setP2Name(const QString& name)
{
    if (activeRecording_ && activeRecording_->playerCount() == 2)
    {
        if (name == "")
            activeRecording_->setPlayerName(1, activeRecording_->playerTag(1));
        else
            activeRecording_->setPlayerName(1, name.toStdString().c_str());

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged,
                            QString(activeRecording_->playerName(1).cStr()));
    }
    else
    {
        p2Name_ = name;
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged, name);
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setGameNumber(uh::GameNumber number)
{
    if (activeRecording_)
    {
        activeRecording_->setGameNumber(number);
        findUniqueGameAndSetNumbers(activeRecording_);

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerSetNumberChanged, activeRecording_->setNumber());
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, activeRecording_->gameNumber());
    }
    else
    {
        gameNumber_ = number;
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, number);
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::tryConnectToServer(const QString& ipAddress, uint16_t port)
{
    tcp_socket sock;
    QByteArray ba = ipAddress.toLocal8Bit();
    if (tcp_socket_connect_to_host(&sock, ba.data(), port) != 0)
    {
        emit failedToConnectToServer();
        return;
    }

    protocol_.reset(new Protocol(sock));
    connect(protocol_.get(), &Protocol::recordingStarted,
            this, &ActiveRecordingManager::onProtocolRecordingStarted);
    connect(protocol_.get(), &Protocol::recordingEnded,
            this, &ActiveRecordingManager::onProtocolRecordingEnded);
    connect(protocol_.get(), &Protocol::serverClosedConnection,
            this, &ActiveRecordingManager::onProtocolConnectionLost);
    protocol_->start();
    emit connectedToServer();
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::disconnectFromServer()
{
    protocol_.reset();
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onProtocolConnectionLost()
{
    protocol_.reset();
    emit disconnectedFromServer();
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onProtocolRecordingStarted(uh::ActiveRecording* recording)
{
    // first off, copy the data we've stored from the UI into the new recording
    // so comparing previous recordings is consistent
    recording->setFormat(format_);
    recording->setGameNumber(gameNumber_);
    recording->setSetNumber(setNumber_);
    if (recording->playerCount() == 2)
    {
        if (p1Name_.length() > 0)
            recording->setPlayerName(0, p1Name_.toStdString().c_str());
        if (p2Name_.length() > 0)
            recording->setPlayerName(1, p2Name_.toStdString().c_str());
    }

    if (shouldStartNewSet(recording))
    {
        recording->setGameNumber(1);
        recording->setSetNumber(1);
        pastRecordings_.clear();
    }
    else
    {
        // Go to the next game in the set
        recording->setGameNumber(recording->gameNumber() + 1);
    }

    // Modify game/set numbers until we have a unique filename
    findUniqueGameAndSetNumbers(recording);

    recording->dispatcher.addListener(this);
    activeRecording_ = recording;
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerRecordingStarted, recording);
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerSetNumberChanged, recording->setNumber());
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, recording->gameNumber());
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerFormatChanged, recording->format());
    if (recording->playerCount() == 2)
    {
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP1NameChanged,
                            QString(recording->playerName(0).cStr()));
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged,
                            QString(recording->playerName(1).cStr()));
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onProtocolRecordingEnded(uh::ActiveRecording* recording)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerRecordingEnded, recording);

    // Save recording
    QFileInfo fileInfo(
        recordingManager_->defaultRecordingSourceDirectory(),
        composeFileName(recording)
    );
    if (recording->saveAs(fileInfo.absoluteFilePath().toStdString().c_str()))
    {
        // Add the new recording to the "All" recording group
        recordingManager_->allRecordingGroup()->addFile(fileInfo.absoluteFilePath());
    }
    else
    {
        // TODO: Need to handle this somehow
    }

    // In between recordings (when players are in the menu) there is no active
    // recording, but it's still possible to edit the names/format/game number/etc
    // so copy the data out of the recording here so it can be edited, and when
    // a new recording starts again we copy the data into the recording.
    format_ = recording->format();
    gameNumber_ = recording->gameNumber();
    setNumber_ = recording->setNumber();
    if (recording->playerCount() == 2)
    {
        p1Name_ = recording->playerName(0).cStr();
        p2Name_ = recording->playerName(1).cStr();
    }

    recording->dispatcher.removeListener(this);
    pastRecordings_.push_back(recording);
    activeRecording_.reset();
}

// ----------------------------------------------------------------------------
bool ActiveRecordingManager::shouldStartNewSet(const uh::ActiveRecording* recording)
{
    // For any game that doesn't have exactly 2 players we don't care about sets
    if (recording->playerCount() != 2)
        return true;

    // No past recordings? -> new set
    if (pastRecordings_.size() == 0)
        return true;
    const auto& prev = pastRecordings_.back();

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
    for (const auto& rec : pastRecordings_)
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
void ActiveRecordingManager::findUniqueGameAndSetNumbers(uh::ActiveRecording* recording)
{
    const QDir& dir = recordingManager_->defaultRecordingSourceDirectory();
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
void ActiveRecordingManager::onRecordingManagerDefaultRecordingLocationChanged(const QDir& path)
{
    (void)path;
    if (activeRecording_)
    {
        findUniqueGameAndSetNumbers(activeRecording_);

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerSetNumberChanged, activeRecording_->setNumber());
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, activeRecording_->gameNumber());
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name)
{
    if (activeRecording_->playerCount() == 2)
    {
        if (player == 0)
            dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP1NameChanged, QString(name.cStr()));
        else
            dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged, QString(name.cStr()));
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingSetNumberChanged(uh::SetNumber number)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingGameNumberChanged(uh::GameNumber number)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingFormatChanged(const uh::SetFormat& format)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state)
{
    (void)player;
    (void)state;
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerPlayerStateAdded, player, state);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onRecordingWinnerChanged(int winner)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerWinnerChanged, winner);
}

}
