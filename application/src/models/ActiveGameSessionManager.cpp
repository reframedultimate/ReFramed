#include "application/Util.hpp"
#include "application/listeners/ActiveGameSessionManagerListener.hpp"
#include "application/models/ActiveGameSessionManager.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/tcp_socket.h"
#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
static QString composeFileName(const rfcommon::Session* session, QString saveFormat)
{
    using namespace rfcommon;
    const SessionMetaData* meta = session->metaData();
    const MappingInfo* map = session->mappingInfo();

    // Create date from metadata if possible.
    const auto stamp = meta->timeStampStarted();
    const uint64_t stampMs = stamp.isValid() ? stamp.millisSinceEpoch() : QDateTime::currentMSecsSinceEpoch();
    const QString date = QDateTime::fromMSecsSinceEpoch(stampMs).toString("yyyy-MM-dd");

    const char* p1char = session->mappingInfo()->fighter.toName(meta->fighterID(0), "(unknown fighter)");
    const char* p2char = session->mappingInfo()->fighter.toName(meta->fighterID(1), "(unknown fighter)");

    if (meta->type() == SessionMetaData::GAME)
    {
        const GameSessionMetaData* gameMeta = static_cast<const GameSessionMetaData*>(meta);

        QString formatDesc = gameMeta->setFormat().description().cStr();
        QString setNumber = QString::number(gameMeta->setNumber().value());
        QString gameNumber = QString::number(gameMeta->gameNumber().value());

        if (meta->fighterCount() == 2)  // 1v1
        {
            saveFormat
                .replace("%date", date)
                .replace("%format", formatDesc)
                .replace("%set", setNumber)
                .replace("%game", gameNumber)
                .replace("%p1name", gameMeta->name(0).cStr())
                .replace("%p2name", gameMeta->name(1).cStr())
                .replace("%p1char", p1char)
                .replace("%p2char", p2char)
                ;
        }
        else
        {
            QStringList playerList;
            for (int i = 0; i < meta->fighterCount(); ++i)
            {
                const char* fighter = map->fighter.toName(meta->fighterID(i), "(unknown fighter)");
                const char* name = meta->name(i).cStr();
                playerList.append(QString(name) + " (" + fighter + ")");
            }
            QString players = playerList.join(" vs ");

            saveFormat = date + " - " + formatDesc + " (" + setNumber + ") - " + players + " Game " + gameNumber;
        }
    }
    else
    {
        assert(meta->type() == SessionMetaData::TRAINING);
        const TrainingSessionMetaData* trainingMeta = static_cast<const TrainingSessionMetaData*>(meta);

        QString sessionNumber = QString::number(trainingMeta->sessionNumber().value());

        saveFormat
                .replace("%date", date)
                .replace("%session", sessionNumber)
                .replace("%p1name", trainingMeta->name(0).cStr())
                .replace("%p1char", p1char)
                .replace("%p2char", p2char)
                ;
    }

    return saveFormat + ".rfr";
}

// ----------------------------------------------------------------------------
ActiveGameSessionManager::ActiveGameSessionManager(Protocol* protocol, ReplayManager* manager, QObject* parent)
    : QObject(parent)
    , protocol_(protocol)
    , replayManager_(manager)
    , gameSaveFormat_("%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game")
    , trainingSaveFormat_("%date - Training - %p1name (%p1char) on %p2char Session %session")
    , format_(rfcommon::SetFormat::FRIENDLIES)
    , gameNumber_(rfcommon::GameNumber::fromValue(1))
    , setNumber_(rfcommon::SetNumber::fromValue(1))
{
    protocol_->dispatcher.addListener(this);
    replayManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveGameSessionManager::~ActiveGameSessionManager()
{
    replayManager_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);

    if (activeSession_)
        activeSession_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::setFormat(const rfcommon::SetFormat& format)
{
    if (activeSession_)
    {
        activeSession_->setFormat(format);
    }
    else
    {
        format_ = format;
    }

    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::setP1Name(const QString& name)
{
    if (activeSession_ && activeSession_->fighterCount() == 2)
    {
        if (name == "")
            activeSession_->setPlayerName(0, activeSession_->tag(0));
        else
            activeSession_->setPlayerName(0, name.toStdString().c_str());

        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged,
                0, activeSession_->name(0));
    }
    else
    {
        p1Name_ = name;
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged, 0, rfcommon::SmallString<15>(name.toStdString().c_str()));
    }
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::setP2Name(const QString& name)
{
    if (activeSession_ && activeSession_->fighterCount() == 2)
    {
        if (name == "")
            activeSession_->setPlayerName(1, activeSession_->tag(1));
        else
            activeSession_->setPlayerName(1, name.toStdString().c_str());

        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged,
                1, activeSession_->name(1));
    }
    else
    {
        p2Name_ = name;
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged, 1, rfcommon::SmallString<15>(name.toStdString().c_str()));
    }
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::setGameNumber(rfcommon::GameNumber number)
{
    if (activeSession_)
    {
        activeSession_->setGameNumber(number);
        findUniqueGameAndSetNumbers(activeSession_);

        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerSetNumberChanged, activeSession_->setNumber());
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameNumberChanged, activeSession_->gameNumber());
    }
    else
    {
        gameNumber_ = number;
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameNumberChanged, number);
    }
}

// ----------------------------------------------------------------------------
bool ActiveGameSessionManager::isSessionRunning() const
{
    return protocol_->isConnected();
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::findUniqueGameAndSetNumbers(rfcommon::Session* session)
{
    const QDir& dir = replayManager_->defaultGameSessionSourceDirectory();
    while (QFileInfo::exists(dir.absoluteFilePath(composeFileName(session))))
    {
        switch (format_.type())
        {
            case rfcommon::SetFormat::FRIENDLIES:
            case rfcommon::SetFormat::PRACTICE:
            case rfcommon::SetFormat::OTHER:
                session->setGameNumber(session->gameNumber() + 1);
                break;

            default:
                session->setSetNumber(session->setNumber() + 1);
                break;
        }
    }
}

// ----------------------------------------------------------------------------
bool ActiveGameSessionManager::shouldStartNewSet(const rfcommon::Session* session)
{
    // For any game that doesn't have exactly 2 players we don't care about sets
    if (session->fighterCount() != 2)
        return true;

    // No past recordings? -> new set
    if (pastSessions_.size() == 0)
        return true;
    const auto& prev = pastSessions_.back();

    // Player tags might have changed
    if (prev->tag(0) != session->tag(0) ||
        prev->tag(1) != session->tag(1))
    {
        return true;
    }

    // Player names might have changed
    if (prev->name(0) != session->name(0) ||
        prev->name(1) != session->name(1))
    {
        return true;
    }

    // Format might have changed
    if (prev->format().type() != session->format().type())
        return true;

    // tally up wins for each player
    int win[2] = {0, 0};
    for (const auto& session : pastSessions_)
        win[session->winner()]++;

    switch (session->format().type())
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
void ActiveGameSessionManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerAttemptConnectToServer, ipAddress, port);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerFailedToConnectToServer, ipAddress, port);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerConnectedToServer, ipAddress, port);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolDisconnectedFromServer()
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerDisconnectedFromServer);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolTrainingStarted(rfcommon::Session* training)
{
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolTrainingResumed(rfcommon::Session* training)
{
}

void ActiveGameSessionManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolTrainingEnded(rfcommon::Session* training)
{
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolGameStarted(rfcommon::Session* game)
{
    // first off, copy the data we've stored from the UI into the new recording
    // so comparing previous recordings is consistent
    game->setFormat(format_);
    game->setGameNumber(gameNumber_);
    game->setSetNumber(setNumber_);
    if (game->fighterCount() == 2)
    {
        if (p1Name_.length() > 0)
            game->setPlayerName(0, p1Name_.toStdString().c_str());
        if (p2Name_.length() > 0)
            game->setPlayerName(1, p2Name_.toStdString().c_str());
    }

    if (shouldStartNewSet(game))
    {
        game->setGameNumber(1);
        game->setSetNumber(1);
        pastSessions_.clear();
    }
    else
    {
        // Go to the next game in the set
        game->setGameNumber(game->gameNumber() + 1);
    }

    // Modify game/set numbers until we have a unique filename
    findUniqueGameAndSetNumbers(game);

    game->dispatcher.addListener(this);
    activeSession_ = game;
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameStarted, game);
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerSetNumberChanged, game->setNumber());
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameNumberChanged, game->gameNumber());
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerFormatChanged, game->format());
    if (game->fighterCount() == 2)
    {
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged,
                0, game->name(0));
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged,
                1, game->name(1));
    }
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolGameResumed(rfcommon::Session* game)
{
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onProtocolGameEnded(rfcommon::Session* game)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameEnded, game);

    // Save as replay
    QDir gamesDir = replayManager_->defaultGameSessionSourceDirectory();
    if (gamesDir.exists() == false)
        gamesDir.mkpath(".");
    QFileInfo fileInfo(gamesDir, composeFileName(game));
    if (game->save(fileInfo.absoluteFilePath().toStdString().c_str()))
    {
        // Add the new recording to the "All" recording group
        replayManager_->allReplayGroup()->addFile(fileInfo.absoluteFilePath());
    }
    else
    {
        // TODO: Need to handle this somehow
    }

    // In between recordings (when players are in the menu) there is no active
    // recording, but it's still possible to edit the names/format/game number/etc
    // so copy the data out of the recording here so it can be edited, and when
    // a new recording starts again we copy the data into the recording.
    format_ = game->format();
    gameNumber_ = game->gameNumber();
    setNumber_ = game->setNumber();
    if (game->fighterCount() == 2)
    {
        p1Name_ = game->name(0).cStr();
        p2Name_ = game->name(1).cStr();
    }

    game->dispatcher.removeListener(this);
    pastSessions_.push_back(game);
    activeSession_.drop();
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path)
{
    (void)path;
    if (activeSession_)
    {
        findUniqueGameAndSetNumbers(activeSession_);

        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerSetNumberChanged, activeSession_->setNumber());
        dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameNumberChanged, activeSession_->gameNumber());
    }
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onSessionMetaDataPlayerNameChanged(int player, const rfcommon::SmallString<15>& name)
{
    if (activeSession_->fighterCount() == 2)
    {
        if (player == 0)
            dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged, 0, name);
        else
            dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerPlayerNameChanged, 1, name);
    }
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onSessionMetaDataSetNumberChanged(rfcommon::SetNumber number)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onSessionMetaDataGameNumberChanged(rfcommon::GameNumber number)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onSessionMetaDataSetFormatChanged(const rfcommon::SetFormat& format)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onFrameDataNewUniqueFrame(int frameIdx, const Frame& frame)
{
    (void)frameIdx; (void)frame;
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onFrameDataNewFrame(int frameIdx, const Frame& frame)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerNewFrame, frameIdx, frame);
}

// ----------------------------------------------------------------------------
void ActiveGameSessionManager::onRunningGameSessionWinnerChanged(int winner)
{
    dispatcher.dispatch(&ActiveGameSessionManagerListener::onActiveGameSessionManagerWinnerChanged, winner);
}

}
