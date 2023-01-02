#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "application/models/ConfigAccessor.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include <QObject>
#include <QDir>
#include <QVector>

namespace rfcommon {
    class FrameData;
    class GameMetadata;
    class Session;
    class TrainingMetadata;
}

namespace rfapp {

class PluginManager;
class Protocol;
class Settings;
class ActiveSessionManagerListener;
class ReplayManager;

/*!
 * \brief Central class that manages set information ("set" is the term used
 * for a set of smash games, e.g. a Bo5 is a set of 3-5 games) and saving
 * sessions to files as they come in.
 */
class ActiveSessionManager
        : public QObject
        , public ReplayManagerListener
        , public rfcommon::ProtocolListener
        , public ConfigAccessor
{
    Q_OBJECT

public:
    ActiveSessionManager(Config* config, Protocol* protocol, ReplayManager* replayManager, PluginManager* pluginManager, QObject* parent=nullptr);
    ~ActiveSessionManager();

    Protocol* protocol() const;

    void setAutoAssociateVideoEnabled(bool enable);
    bool autoAssociateVideoEnabled() const;

    void setAutoAssociateVideoDirectory(const QString& dir);
    const QString& autoAssociateVideoDirectory() const;

    void setAutoAssociateVideoFrameOffset(int offset);
    int autoAssociateVideoFrameOffset() const;

    rfcommon::ListenerDispatcher<ActiveSessionManagerListener> dispatcher;

private:
    void updateConfig();
    bool findFreeRoundAndGameNumbers(rfcommon::MappingInfo* map, rfcommon::Metadata* mdata);

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::Session* training) override;
    void onProtocolTrainingResumed(rfcommon::Session* training) override;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::Session* training) override;
    void onProtocolGameStarted(rfcommon::Session* game) override;
    void onProtocolGameResumed(rfcommon::Session* game) override;
    void onProtocolGameEnded(rfcommon::Session* game) override;

private:
    void onReplayManagerDefaultGamePathChanged(const QDir& path) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override;
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override;
    void onReplayManagerGroupRemoved(ReplayGroup* group) override;

    void onReplayManagerGamePathAdded(const QDir& path) override;
    void onReplayManagerGamePathRemoved(const QDir& path) override;

    void onReplayManagerVideoPathAdded(const QDir& path) override;
    void onReplayManagerVideoPathRemoved(const QDir& path) override;

private:
    Protocol* protocol_;
    ReplayManager* replayManager_;
    PluginManager* pluginManager_;
    QString autoAssociateVideoDir_;
    QVector<rfcommon::Reference<rfcommon::GameMetadata>> pastMetadata_;
    int autoAssociateVideoFrameOffset_ = 0;
    bool autoAssociateVideoEnabled_ = false;
};

}
