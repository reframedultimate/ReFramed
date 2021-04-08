#include "uh/listeners/ActiveRecordingManagerListener.hpp"
#include "uh/models/ActiveRecordingManager.hpp"
#include "uh/models/Settings.hpp"
#include "uh/platform/tcp_socket.h"

namespace uh {

// ----------------------------------------------------------------------------
ActiveRecordingManager::ActiveRecordingManager(Settings* settings, QObject *parent)
    : QObject(parent)
    , savePath_(settings->activeRecordingSavePath)
{
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setFormat(SetFormat format, const QString& otherFormatDesc)
{
    if (activeRecording_)
    {
        activeRecording_->setFormat(format, otherFormatDesc);
    }
    else
    {
        format_ = format;
        if (static_cast<SetFormat>(format) == SetFormat::OTHER)
            otherFormatDesc_ = otherFormatDesc;
    }

    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerFormatChanged, format, otherFormatDesc);
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setP1Name(const QString& name)
{
    if (activeRecording_ && activeRecording_->playerCount() == 2)
    {
        if (name == "")
            activeRecording_->setPlayerName(0, activeRecording_->playerTag(0));
        else
            activeRecording_->setPlayerName(0, name);

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP1NameChanged, activeRecording_->playerName(0));
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
            activeRecording_->setPlayerName(1, name);

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged, activeRecording_->playerName(1));
    }
    else
    {
        p2Name_ = name;
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged, name);
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::setGameNumber(int number)
{
    if (activeRecording_)
    {
        activeRecording_->setGameNumber(number);
        findUniqueGameAndSetNumbers(activeRecording_.data());

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
void ActiveRecordingManager::setSavePath(const QDir& savePath)
{
    savePath_ = savePath;

    if (activeRecording_)
    {
        findUniqueGameAndSetNumbers(activeRecording_.data());

        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerSetNumberChanged, activeRecording_->setNumber());
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, activeRecording_->gameNumber());
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
    connect(protocol_.get(), SIGNAL(recordingStarted(ActiveRecording*)), this, SLOT(onProtocolRecordingStarted(ActiveRecording*)));
    connect(protocol_.get(), SIGNAL(recordingEnded(ActiveRecording*)), this, SLOT(onProtocolRecordingEnded(ActiveRecording*)));
    connect(protocol_.get(), SIGNAL(serverClosedConnection()), this, SLOT(onProtocolConnectionLost()));
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
void ActiveRecordingManager::onProtocolRecordingStarted(ActiveRecording* recording)
{
    // first off, copy the data we've stored from the UI into the new recording
    // so comparing previous recordings is consistent
    recording->setFormat(format_, otherFormatDesc_);
    recording->setGameNumber(gameNumber_);
    recording->setSetNumber(setNumber_);
    if (recording->playerCount() == 2)
    {
        if (p1Name_.length() > 0)
            recording->setPlayerName(0, p1Name_);
        if (p2Name_.length() > 0)
            recording->setPlayerName(1, p2Name_);
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
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerFormatChanged, recording->format(), recording->formatDesc());
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerGameNumberChanged, recording->gameNumber());
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerSetNumberChanged, recording->setNumber());
    if (recording->playerCount() == 2)
    {
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP1NameChanged, recording->playerName(0));
        dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerP2NameChanged, recording->playerName(1));
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onProtocolRecordingEnded(ActiveRecording* recording)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerRecordingEnded, recording);

    recording->saveAs(savePath_.absoluteFilePath(composeFileName(recording)));

    format_ = recording->format();
    gameNumber_ = recording->gameNumber();
    setNumber_ = recording->setNumber();
    if (recording->playerCount() == 2)
    {
        p1Name_ = recording->playerName(0);
        p2Name_ = recording->playerName(1);
    }
    if (format_ == SetFormat::OTHER)
        otherFormatDesc_ = recording->formatDesc();

    recording->dispatcher.removeListener(this);
    pastRecordings_.push_back(QExplicitlySharedDataPointer<ActiveRecording>(recording));
    activeRecording_.reset();
}

// ----------------------------------------------------------------------------
QString ActiveRecordingManager::composeFileName(const ActiveRecording* recording) const
{
    QString date = recording->timeStarted().toString("yyyy-MM-dd");
    QStringList playerList;
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        const QString* fighterName = recording->mappingInfo().fighterID.map(
                    recording->playerFighterID(i));
        if (fighterName)
            playerList.append(recording->playerName(i) + " (" + *fighterName + ")");
        else
            playerList.append(recording->playerName(i));
    }
    QString players = playerList.join(" vs ");

    if (recording->setNumber() == 1)
        return date + " - " + recording->formatDesc() + " - " + players + " Game " + QString::number(recording->gameNumber()) + ".uhr";

    return date + " - " + recording->formatDesc() + " (" + QString::number(recording->setNumber()) + ") - " + players + " Game " + QString::number(recording->gameNumber()) + ".uhr";
}

// ----------------------------------------------------------------------------
bool ActiveRecordingManager::shouldStartNewSet(const ActiveRecording* recording)
{
    // For any game that doesn't have exactly 2 players we don't care about sets
    if (recording->playerCount() != 2)
        return true;

    // No past recordings? -> new set
    if (pastRecordings_.length() == 0)
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
    if (prev->format() != recording->format())
        return true;

    // tally up wins for each player
    int win[2] = {0, 0};
    for (const auto& rec : pastRecordings_)
        win[rec->winner()]++;

    switch (recording->format())
    {
        case SetFormat::BO3: {
            if (win[0] >= 2 || win[1] >= 2)
                return true;
        } break;

        case SetFormat::BO5: {
            if (win[0] >= 3 || win[1] >= 3)
                return true;
        } break;

        case SetFormat::BO7: {
            if (win[0] >= 4 || win[1] >= 4)
                return true;
        } break;

        case SetFormat::FT5: {
            if (win[0] >= 5 || win[1] >= 5)
                return true;
        } break;

        case SetFormat::FT10: {
            if (win[0] >= 10 || win[1] >= 10)
                return true;
        } break;

        case SetFormat::FRIENDLIES:
        case SetFormat::PRACTICE:
        case SetFormat::OTHER:
            break;
    }

    return false;
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::findUniqueGameAndSetNumbers(ActiveRecording* recording)
{
    while (QFileInfo::exists(savePath_.absoluteFilePath(composeFileName(recording))))
    {
        switch (format_)
        {
            case SetFormat::FRIENDLIES:
            case SetFormat::PRACTICE:
            case SetFormat::OTHER:
                recording->setGameNumber(recording->gameNumber() + 1);
                break;

            default:
                recording->setSetNumber(recording->setNumber() + 1);
                break;
        }
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingManager::onActiveRecordingPlayerStateAdded(int player, const PlayerState& state)
{
    dispatcher.dispatch(&ActiveRecordingManagerListener::onActiveRecordingManagerPlayerStateAdded, player, state);
}

}
