#pragma once

#include "application/listeners/ListenerDispatcher.hpp"
#include "application/listeners/RecordingListener.hpp"
#include "application/listeners/RecordingManagerListener.hpp"
#include "application/models/SetFormat.hpp"
#include "application/models/ActiveRecording.hpp"  // MOC requires this because of smart pointers
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "application/models/PlayerState.hpp"  // MOC requires this because of smart pointers
#include <QObject>
#include <QDir>

namespace uh {

class Protocol;
class Settings;
class ActiveRecording;
class ActiveRecordingManagerListener;
class RecordingManager;

/*!
 * \brief Central class that controls the connection + protocol with the Switch
 * and manages set information and saving recordings to files as they come in.
 */
class ActiveRecordingManager : public QObject
                             , public RecordingManagerListener
                             , public RecordingListener
{
    Q_OBJECT

public:
    ActiveRecordingManager(RecordingManager* recordingManager, QObject* parent=nullptr);
    ~ActiveRecordingManager();

    void setFormat(const SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(int number);

    ListenerDispatcher<ActiveRecordingManagerListener> dispatcher;

signals:
    void failedToConnectToServer();
    void connectedToServer();
    void disconnectedFromServer();

public slots:
    void tryConnectToServer(const QString& ipAddress, uint16_t port);
    void disconnectFromServer();

private slots:
    void onProtocolConnectionLost();
    void onProtocolRecordingStarted(ActiveRecording* recording);
    void onProtocolRecordingEnded(ActiveRecording* recording);

private:
    void findUniqueGameAndSetNumbers(ActiveRecording* recording);
    bool shouldStartNewSet(const ActiveRecording* recording);
    QString composeFileName(const ActiveRecording* recording) const;

private:
    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override;

    void onRecordingManagerGroupAdded(RecordingGroup* group) override { (void)group; }
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override { (void)group; }

    void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerRecordingSourceRemoved(const QString& name) override { (void)name; }

    void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onActiveRecordingPlayerNameChanged(int player, const QString& name) override;
    void onActiveRecordingSetNumberChanged(int number) override;
    void onActiveRecordingGameNumberChanged(int number) override;
    void onActiveRecordingFormatChanged(const SetFormat& format) override;
    void onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state) override;
    void onActiveRecordingNewPlayerState(int player, const PlayerState& state) override;
    void onRecordingWinnerChanged(int winner) override;

private:
    QScopedPointer<Protocol> protocol_;
    QVector<QExplicitlySharedDataPointer<ActiveRecording>> pastRecordings_;
    QExplicitlySharedDataPointer<ActiveRecording> activeRecording_;
    RecordingManager* recordingManager_;
    QString p1Name_;
    QString p2Name_;
    SetFormat format_;
    int gameNumber_ = 1;
    int setNumber_ = 1;
};

}
