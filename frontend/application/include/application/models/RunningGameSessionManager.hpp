#pragma once

#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "uh/Reference.hpp"
#include "uh/SetFormat.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/SessionListener.hpp"
#include "uh/RunningGameSession.hpp"  // MOC requires this because of smart pointers
#include "uh/PlayerState.hpp"  // MOC requires this because of smart pointers
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
                                , public uh::SessionListener
{
    Q_OBJECT

public:
    RunningGameSessionManager(ReplayManager* manager, QObject* parent=nullptr);
    ~RunningGameSessionManager();

    void tryConnectToServer(const QString& ipAddress, uint16_t port);
    void disconnectFromServer();
    bool isSessionRunning() const;

    void setFormat(const uh::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(uh::GameNumber number);

    uh::ListenerDispatcher<RunningGameSessionManagerListener> dispatcher;

private slots:
    void onProtocolConnectionLost();
    void onProtocolRecordingStarted(uh::RunningGameSession* recording);
    void onProtocolRecordingEnded(uh::RunningGameSession* recording);

private:
    void findUniqueGameAndSetNumbers(uh::RunningGameSession* recording);
    bool shouldStartNewSet(const uh::RunningGameSession* recording);

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

    void onRunningSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int player, const uh::PlayerState& state) override;

private:
    std::unique_ptr<Protocol> protocol_;
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
