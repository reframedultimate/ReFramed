#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "rfcommon/Reference.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/SessionMetaDataListener.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include <QObject>
#include <QDir>
#include <vector>

namespace rfcommon {
    class Session;
}

namespace rfapp {

class Protocol;
class Settings;
class RunningGameSessionManagerListener;
class ReplayManager;

/*!
 * \brief Central class that controls the connection + protocol with the Switch
 * and manages set information and saving recordings to files as they come in.
 */
class RunningGameSessionManager : public QObject
                                , public ReplayManagerListener
                                , public rfcommon::ProtocolListener
                                , public rfcommon::SessionMetaDataListener,
                                , public rfcommon::FrameDataListener
{
    Q_OBJECT

public:
    RunningGameSessionManager(Protocol* protocol, ReplayManager* manager, QObject* parent=nullptr);
    ~RunningGameSessionManager();

    void setFormat(const rfcommon::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(rfcommon::GameNumber number);

    bool isSessionRunning() const;

    rfcommon::ListenerDispatcher<RunningGameSessionManagerListener> dispatcher;

private:
    void findUniqueGameAndSetNumbers(rfcommon::RunningGameSession* recording);
    bool shouldStartNewSet(const rfcommon::RunningGameSession* recording);

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training) override;
    void onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training) override;
    void onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldTraining, rfcommon::RunningTrainingSession* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training) override;
    void onProtocolMatchStarted(rfcommon::RunningGameSession* match) override;
    void onProtocolMatchResumed(rfcommon::RunningGameSession* match) override;
    void onProtocolMatchEnded(rfcommon::RunningGameSession* match) override;

private:
    void onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override { (void)group; }
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override { (void)group; (void)oldName; (void)newName; }
    void onReplayManagerGroupRemoved(ReplayGroup* group) override { (void)group; }

    void onReplayManagerReplaySourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerReplaySourceRemoved(const QString& name) override { (void)name; }

    void onReplayManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onSessionMetaDataPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onSessionMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onSessionMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onSessionMetaDataFormatChanged(const rfcommon::SetFormat& format) override;
    void onSessionMetaDataWinnerChanged(int winner) override;

    void onFrameDataNewUniqueFrame(int frameIdx, const Frame& frame) override;
    void onFrameDataNewFrame(int frameIdx, const Frame& frame) override;

private:
    Protocol* protocol_;
    std::vector<rfcommon::Reference<rfcommon::Session>> pastSessions_;
    rfcommon::Reference<rfcommon::Session> activeSession_;
    ReplayManager* savedSessionManager_;
    QString p1Name_;
    QString p2Name_;
    rfcommon::SetFormat format_;
    rfcommon::GameNumber gameNumber_ = 1;
    rfcommon::SetNumber setNumber_ = 1;
};

}
