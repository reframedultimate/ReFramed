#include "application/Util.hpp"
#include "application/listeners/ActiveSessionManagerListener.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/models/AutoAssociateVideosTask.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TrainingMetaData.hpp"

#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
ActiveSessionManager::ActiveSessionManager(Protocol* protocol, ReplayManager* replayManager, PluginManager* pluginManager, QObject* parent)
    : QObject(parent)
    , protocol_(protocol)
    , replayManager_(replayManager)
    , pluginManager_(pluginManager)
{
    protocol_->dispatcher.addListener(this);
    replayManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionManager::~ActiveSessionManager()
{
    replayManager_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
Protocol* ActiveSessionManager::protocol() const
{
    PROFILE(ActiveSessionManager, protocol);

    return protocol_;
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setAutoAssociateVideoDirectory(const QString& dir)
{
    autoAssociateVideoDir_ = dir;
}

// ----------------------------------------------------------------------------
const QString& ActiveSessionManager::autoAssociateVideoDirectory() const
{
    return autoAssociateVideoDir_;
}

// ----------------------------------------------------------------------------
bool ActiveSessionManager::findFreeRoundAndGameNumbers(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    const QDir& dir = replayManager_->defaultGamePath();
    while (true)
    {
        QString fileName = rfcommon::ReplayFileParts::fromMetaData(map, mdata).toFileName().cStr();
        if (fileName == "")
            return false;
        if (QFileInfo::exists(dir.absoluteFilePath(fileName)) == false)
            return true;

        switch (mdata->type())
        {
            case rfcommon::MetaData::GAME: {
                auto gameMeta = mdata->asGame();
                const auto sessionNumber = gameMeta->round().number();
                gameMeta->setRound(rfcommon::Round::fromType(gameMeta->round().type(), sessionNumber + 1));
            } break;

            case rfcommon::MetaData::TRAINING: {
                auto trainingMeta = mdata->asTraining();
                trainingMeta->setSessionNumber(trainingMeta->sessionNumber() + 1);
            } break;
        }
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void ActiveSessionManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void ActiveSessionManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void ActiveSessionManager::onProtocolDisconnectedFromServer() {}
void ActiveSessionManager::onProtocolTrainingStarted(rfcommon::Session* training) {}
void ActiveSessionManager::onProtocolTrainingResumed(rfcommon::Session* training) {}
void ActiveSessionManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void ActiveSessionManager::onProtocolTrainingEnded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameStarted);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameStarted, game);

    // Modify game/set numbers until we have a unique filename
    findFreeRoundAndGameNumbers(game->tryGetMappingInfo(), game->tryGetMetaData());
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameResumed);

    ActiveSessionManager::onProtocolGameStarted(game);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameEnded(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameEnded);

    auto saveGame = [this, game] {
        // Save as replay. This will also add the session to the "All" replay group
        if (replayManager_->saveReplayWithDefaultSettings(game) == false)
        {
            // TODO need to handle this somehow
        }
    };

    if (autoAssociateVideoDir_.isEmpty())
    {
        saveGame();
    }
    else
    {
        AutoAssociateVideoTask* task = new AutoAssociateVideoTask(
                    game, autoAssociateVideoDir_,
                    pluginManager_,
                    rfcommon::Log::root()->child("AutoAssociateVideoTask"));

        // Delete task object when thread is done
        connect(task, &QThread::finished, task, &QObject::deleteLater);

        // Save game as replay once task completes, regardless of success
        connect(task, &AutoAssociateVideoTask::success, saveGame);
        connect(task, &AutoAssociateVideoTask::failure, saveGame);

        task->start();
    }

    /*
    // In between sessions (when players are in the menu) there is no active
    // session, but it's still possible to edit the names/format/game number/etc
    // so copy the data out of the session here so it can be edited, and when
    // a new session starts again we copy the data into the session.
    auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
    format_ = meta->setFormat();
    gameNumber_ = meta->gameNumber();
    setNumber_ = meta->setNumber();
    if (meta->fighterCount() == 2)
    {
        p1Name_ = meta->name(0).cStr();
        p2Name_ = meta->name(1).cStr();
    }

    activeFrameData_->dispatcher.removeListener(this);
    activeMetaData_->dispatcher.removeListener(this);
    pastGameMetaData_.push_back(meta);
    activeMappingInfo_.drop();
    activeMetaData_.drop();

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameEnded, game);*/
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onReplayManagerDefaultGamePathChanged(const QDir& path)
{
    PROFILE(ActiveSessionManager, onReplayManagerDefaultGamePathChanged);
/*
    (void)path;
    if (activeMetaData_.isNull())
        return;

    switch (activeMetaData_->type())
    {
        case rfcommon::MetaData::GAME: {
            auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
            replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetNumberChanged, meta->setNumber());
            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, meta->gameNumber());
        } break;

        case rfcommon::MetaData::TRAINING: {
            auto meta = static_cast<rfcommon::TrainingMetaData*>(activeMetaData_.get());
            replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingSessionNumberChanged, meta->sessionNumber());
        } break;
    }*/
}

void ActiveSessionManager::onReplayManagerGroupAdded(ReplayGroup* group) {}
void ActiveSessionManager::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) {}
void ActiveSessionManager::onReplayManagerGroupRemoved(ReplayGroup* group) {}

void ActiveSessionManager::onReplayManagerGamePathAdded(const QDir& path) {}
void ActiveSessionManager::onReplayManagerGamePathRemoved(const QDir& path) {}

void ActiveSessionManager::onReplayManagerVideoPathAdded(const QDir& path) {}
void ActiveSessionManager::onReplayManagerVideoPathRemoved(const QDir& path) {}

}
