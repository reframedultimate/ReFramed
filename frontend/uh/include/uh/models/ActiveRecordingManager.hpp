#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include "uh/listeners/RecordingListener.hpp"
#include "uh/models/SetFormat.hpp"
#include "uh/models/ActiveRecording.hpp"  // MOC requires this because of smart pointers
#include "uh/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "uh/models/PlayerState.hpp"  // MOC requires this because of smart pointers
#include <QObject>
#include <QDir>

namespace uh {

class Protocol;
class Settings;
class ActiveRecording;
class ActiveRecordingManagerListener;

/*!
 * \brief Central class that controls the connection + protocol with the Switch
 * and manages set information and saving recordings to files as they come in.
 */
class ActiveRecordingManager : public QObject
                             , public RecordingListener
{
    Q_OBJECT

public:
    ActiveRecordingManager(Settings* settings, QObject* parent=nullptr);
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
    void setSavePath(const QDir& savePath);
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
    QDir savePath_;
    QString p1Name_;
    QString p2Name_;
    SetFormat format_;
    int gameNumber_ = 1;
    int setNumber_ = 1;
};

}
