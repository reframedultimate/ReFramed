#pragma once

#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "uh/Reference.hpp"
#include "uh/SetFormat.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/PlayerState.hpp"  // MOC requires this because of smart pointers
#include "uh/ProtocolListener.hpp"
#include "uh/RunningGameSession.hpp"  // MOC requires this because of smart pointers
#include "uh/SessionListener.hpp"
#include <QObject>
#include <QDir>
#include <vector>

namespace uhapp {

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
                                , public uh::ProtocolListener
                                , public uh::SessionListener
{
    Q_OBJECT

public:
    RunningGameSessionManager(Protocol* protocol, ReplayManager* manager, QObject* parent=nullptr);
    ~RunningGameSessionManager();

    void setFormat(const uh::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(uh::GameNumber number);

    bool isSessionRunning() const;

    uh::ListenerDispatcher<RunningGameSessionManagerListener> dispatcher;

private:
    void findUniqueGameAndSetNumbers(uh::RunningGameSession* recording);
    bool shouldStartNewSet(const uh::RunningGameSession* recording);

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(uh::RunningTrainingSession* session) override;
    void onProtocolTrainingEnded(uh::RunningTrainingSession* session) override;
    void onProtocolMatchStarted(uh::RunningGameSession* session) override;
    void onProtocolMatchEnded(uh::RunningGameSession* session) override;

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
    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override;
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override;
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) override;
    void onRunningGameSessionWinnerChanged(int winner) override;

    void onRunningTrainingSessionTrainingReset() override {}

    void onRunningSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int player, const uh::PlayerState& state) override;

private:
    Protocol* protocol_;
    std::vector<uh::Reference<uh::RunningGameSession>> pastSessions_;
    uh::Reference<uh::RunningGameSession> activeSession_;
    ReplayManager* savedSessionManager_;
    QString p1Name_;
    QString p2Name_;
    uh::SetFormat format_;
    uh::GameNumber gameNumber_ = 1;
    uh::SetNumber setNumber_ = 1;
};

}
