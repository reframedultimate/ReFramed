#include "application/Util.hpp"
#include "application/listeners/ActiveSessionManagerListener.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/models/AutoAssociateVideosTask.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/ReplayFilename.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TrainingMetadata.hpp"

#include <QDateTime>

namespace rfapp {

using namespace nlohmann;

// ----------------------------------------------------------------------------
ActiveSessionManager::ActiveSessionManager(Config* config, Protocol* protocol, ReplayManager* replayManager, PluginManager* pluginManager, QObject* parent)
    : QObject(parent)
    , ConfigAccessor(config)
    , protocol_(protocol)
    , replayManager_(replayManager)
    , pluginManager_(pluginManager)
{
    json& cfg = configRoot();
    json& jActiveSessionManager = cfg["activesessionmanager"];
    json& jAutoAssociateVideos = jActiveSessionManager["autoassociatevideos"];
    json& jEnable = jAutoAssociateVideos["enable"];
    json& jDir = jAutoAssociateVideos["dir"];
    json& jOffset = jAutoAssociateVideos["offset"];
    if (jEnable.is_boolean())
        autoAssociateVideoEnabled_ = jEnable.get<bool>();
    if (jDir.is_string())
        autoAssociateVideoDir_ = QString::fromUtf8(jDir.get<std::string>().c_str());
    if (jOffset.is_number_integer())
        autoAssociateVideoFrameOffset_ = jOffset.get<int>();

    protocol_->dispatcher.addListener(this);
    replayManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionManager::~ActiveSessionManager()
{
    replayManager_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);

    json& cfg = configRoot();
    json& jActiveSessionManager = cfg["activesessionmanager"];
    json& jAutoAssociateVideos = jActiveSessionManager["autoassociatevideos"];
    jAutoAssociateVideos["enable"] = autoAssociateVideoEnabled_;
    jAutoAssociateVideos["dir"] = autoAssociateVideoDir_.toUtf8().constData();
    jAutoAssociateVideos["offset"] = autoAssociateVideoFrameOffset_;
}

// ----------------------------------------------------------------------------
Protocol* ActiveSessionManager::protocol() const
{
    PROFILE(ActiveSessionManager, protocol);

    return protocol_;
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setAutoAssociateVideoEnabled(bool enable)
{
    PROFILE(ActiveSessionManager, setAutoAssociateVideoEnabled);

    autoAssociateVideoEnabled_ = enable;
}

// ----------------------------------------------------------------------------
bool ActiveSessionManager::autoAssociateVideoEnabled() const
{
    return autoAssociateVideoEnabled_;
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setAutoAssociateVideoDirectory(const QString& dir)
{
    PROFILE(ActiveSessionManager, setAutoAssociateVideoDirectory);

    autoAssociateVideoDir_ = dir;
}

// ----------------------------------------------------------------------------
const QString& ActiveSessionManager::autoAssociateVideoDirectory() const
{
    PROFILE(ActiveSessionManager, autoAssociateVideoDirectory);

    return autoAssociateVideoDir_;
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setAutoAssociateVideoFrameOffset(int offset)
{
    autoAssociateVideoFrameOffset_ = offset;
}

// ----------------------------------------------------------------------------
int ActiveSessionManager::autoAssociateVideoFrameOffset() const
{
    return autoAssociateVideoFrameOffset_;
}

// ----------------------------------------------------------------------------
bool ActiveSessionManager::findFreeRoundAndGameNumbers(rfcommon::MappingInfo* map, rfcommon::Metadata* mdata)
{
    PROFILE(ActiveSessionManager, findFreeRoundAndGameNumbers);

    const QDir& dir = replayManager_->defaultGamePath();
    while (true)
    {
        QString fileName = QString::fromUtf8(rfcommon::ReplayFilename::fromMetadata(map, mdata).cStr());
        if (fileName.isEmpty())
            return false;
        if (QFileInfo::exists(dir.absoluteFilePath(fileName)) == false)
            return true;

        switch (mdata->type())
        {
            case rfcommon::Metadata::GAME: {
                auto gameMeta = mdata->asGame();
                const auto sessionNumber = gameMeta->round().number();
                gameMeta->setRound(rfcommon::Round::fromType(gameMeta->round().type(), sessionNumber + 1));
            } break;

            case rfcommon::Metadata::TRAINING: {
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
    findFreeRoundAndGameNumbers(game->tryGetMappingInfo(), game->tryGetMetadata());
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

    if (autoAssociateVideoEnabled_ == false || autoAssociateVideoDir_.isEmpty())
    {
        saveGame();
    }
    else
    {
        AutoAssociateVideoTask* task = new AutoAssociateVideoTask(
                    game, autoAssociateVideoDir_, autoAssociateVideoFrameOffset_,
                    pluginManager_,
                    rfcommon::Log::root()->child("AutoAssociateVideoTask"));

        // Delete task object when thread is done
        connect(task, &QThread::finished, task, &QObject::deleteLater);

        // Save game as replay once task completes, regardless of success
        connect(task, &AutoAssociateVideoTask::success, saveGame);
        connect(task, &AutoAssociateVideoTask::failure, saveGame);

        connect(task, &AutoAssociateVideoTask::success, [this] {
            replayManager_->addVideoPath(autoAssociateVideoDir_);
        });

        task->start();
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onReplayManagerDefaultGamePathChanged(const QDir& path)
{
    PROFILE(ActiveSessionManager, onReplayManagerDefaultGamePathChanged);
}

void ActiveSessionManager::onReplayManagerGroupAdded(ReplayGroup* group) {}
void ActiveSessionManager::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) {}
void ActiveSessionManager::onReplayManagerGroupRemoved(ReplayGroup* group) {}

void ActiveSessionManager::onReplayManagerGamePathAdded(const QDir& path) {}
void ActiveSessionManager::onReplayManagerGamePathRemoved(const QDir& path) {}

void ActiveSessionManager::onReplayManagerVideoPathAdded(const QDir& path) {}
void ActiveSessionManager::onReplayManagerVideoPathRemoved(const QDir& path) {}

}
