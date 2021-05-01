#pragma once

#include "application/listeners/RecordingManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "uh/Reference.hpp"
#include "uh/SetFormat.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/ActiveRecording.hpp"  // MOC requires this because of smart pointers
#include "uh/PlayerState.hpp"  // MOC requires this because of smart pointers
#include <QObject>
#include <QDir>
#include <vector>

namespace uhapp {

class Protocol;
class Settings;
class ActiveRecordingManagerListener;
class RecordingManager;

/*!
 * \brief Central class that controls the connection + protocol with the Switch
 * and manages set information and saving recordings to files as they come in.
 */
class ActiveRecordingManager : public QObject
                             , public RecordingManagerListener
                             , public uh::RecordingListener
{
    Q_OBJECT

public:
    ActiveRecordingManager(RecordingManager* recordingManager, QObject* parent=nullptr);
    ~ActiveRecordingManager();

    void setFormat(const uh::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(uh::GameNumber number);

    uh::ListenerDispatcher<ActiveRecordingManagerListener> dispatcher;

signals:
    void failedToConnectToServer();
    void connectedToServer();
    void disconnectedFromServer();

public slots:
    void tryConnectToServer(const QString& ipAddress, uint16_t port);
    void disconnectFromServer();

private slots:
    void onProtocolConnectionLost();
    void onProtocolRecordingStarted(uh::ActiveRecording* recording);
    void onProtocolRecordingEnded(uh::ActiveRecording* recording);

private:
    void findUniqueGameAndSetNumbers(uh::ActiveRecording* recording);
    bool shouldStartNewSet(const uh::ActiveRecording* recording);

private:
    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override;

    void onRecordingManagerGroupAdded(RecordingGroup* group) override { (void)group; }
    void onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName) override { (void)group; (void)oldName; (void)newName; }
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override { (void)group; }

    void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerRecordingSourceRemoved(const QString& name) override { (void)name; }

    void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onActiveRecordingSetNumberChanged(uh::SetNumber number) override;
    void onActiveRecordingGameNumberChanged(uh::GameNumber number) override;
    void onActiveRecordingFormatChanged(const uh::SetFormat& format) override;
    void onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state) override;
    void onRecordingWinnerChanged(int winner) override;

private:
    std::unique_ptr<Protocol> protocol_;
    std::vector<uh::Reference<uh::ActiveRecording>> pastRecordings_;
    uh::Reference<uh::ActiveRecording> activeRecording_;
    RecordingManager* recordingManager_;
    QString p1Name_;
    QString p2Name_;
    uh::SetFormat format_;
    uh::GameNumber gameNumber_ = 1;
    uh::SetNumber setNumber_ = 1;
};

}
