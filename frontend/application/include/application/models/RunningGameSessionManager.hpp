#pragma once

#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "uh/Reference.hpp"
#include "uh/SetFormat.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/RunningGameSessionListener.hpp"
#include "uh/RunningGameSession.hpp"  // MOC requires this because of smart pointers
#include "uh/PlayerState.hpp"  // MOC requires this because of smart pointers
#include <QObject>
#include <QDir>
#include <vector>

namespace uhapp {

class Protocol;
class Settings;
class RunningGameSessionManagerListener;
class SavedGameSessionManager;

/*!
 * \brief Central class that controls the connection + protocol with the Switch
 * and manages set information and saving recordings to files as they come in.
 */
class RunningGameSessionManager : public QObject
                                , public SavedGameSessionManagerListener
                                , public uh::RunningGameSessionListener
{
    Q_OBJECT

public:
    RunningGameSessionManager(SavedGameSessionManager* recordingManager, QObject* parent=nullptr);
    ~RunningGameSessionManager();

    void setFormat(const uh::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(uh::GameNumber number);

    uh::ListenerDispatcher<RunningGameSessionManagerListener> dispatcher;

signals:
    void failedToConnectToServer();
    void connectedToServer();
    void disconnectedFromServer();

public slots:
    void tryConnectToServer(const QString& ipAddress, uint16_t port);
    void disconnectFromServer();

private slots:
    void onProtocolConnectionLost();
    void onProtocolRecordingStarted(uh::RunningGameSession* recording);
    void onProtocolRecordingEnded(uh::RunningGameSession* recording);

private:
    void findUniqueGameAndSetNumbers(uh::RunningGameSession* recording);
    bool shouldStartNewSet(const uh::RunningGameSession* recording);

private:
    void onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged(const QDir& path) override;

    void onSavedGameSessionManagerGroupAdded(RecordingGroup* group) override { (void)group; }
    void onSavedGameSessionManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName) override { (void)group; (void)oldName; (void)newName; }
    void onSavedGameSessionManagerGroupRemoved(RecordingGroup* group) override { (void)group; }

    void onSavedGameSessionManagerGameSessionSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onSavedGameSessionManagerGameSessionSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onSavedGameSessionManagerGameSessionSourceRemoved(const QString& name) override { (void)name; }

    void onSavedGameSessionManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onSavedGameSessionManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onSavedGameSessionManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override;
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override;
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) override;
    void onRunningGameSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onRunningGameSessionNewPlayerState(int player, const uh::PlayerState& state) override;
    void onRecordingWinnerChanged(int winner) override;

private:
    std::unique_ptr<Protocol> protocol_;
    std::vector<uh::Reference<uh::RunningGameSession>> pastRecordings_;
    uh::Reference<uh::RunningGameSession> activeRecording_;
    SavedGameSessionManager* recordingManager_;
    QString p1Name_;
    QString p2Name_;
    uh::SetFormat format_;
    uh::GameNumber gameNumber_ = 1;
    uh::SetNumber setNumber_ = 1;
};

}
