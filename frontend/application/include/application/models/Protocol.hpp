#pragma once

#include "uh/tcp_socket.h"
#include "uh/Reference.hpp"
#include "uh/PlayerState.hpp"  // Required by moc_Protocol.cpp
#include "uh/ListenerDispatcher.hpp"

#include <QObject>

namespace uh {
    class MappingInfo;
    class ProtocolListener;
    class RunningSession;
    class RunningGameSession;
    class RunningTrainingSession;
}

namespace uhapp {

class ProtocolConnectTask;
class ProtocolCommunicateTask;

/*!
 * \brief Decodes the incoming stream from the nintendo switch into structures.
 *
 * When initially connecting, the switch will send tables containing information
 * on which enum values map to what names. The switch only sends this information
 * once, but it is stored by the Protocol class and emitted every time a new
 * match starts, or when training mode is entered.
 *
 * Under normal operation, the sequence of events will be:
 *   1) Single matchStarted() call
 *   2) Multiple newPlayerState() calls
 *   3) Single matchEnded() call
 *
 * For training mode, the sequence of events will be:
 *   1) Single trainingStarted() call
 *   2) Multiple newPlayerState() calls and trainingReset() calls
 *   3) Single trainingEnded() call
 *
 * If the client prematurely disconnects, matchEnded() will still be emitted
 * before connectionClosed() (if a match is in progress). If no match is in
 * progress, then only connectionClosed() is emitted. Similarly, trainingEnded()
 * will still be emitted if training was in progress.
 */
class Protocol : public QObject
{
    Q_OBJECT

public:
    explicit Protocol(QObject* parent=nullptr);
    ~Protocol();

    void connectToServer(const QString& ipAddress, uint16_t port);
    void disconnectFromServer();

    bool isTryingToConnect() const;
    bool isConnected() const;

    uh::ListenerDispatcher<uh::ProtocolListener> dispatcher;

private slots:
    void onConnectionSuccess(tcp_socket socket, const QString& ipAddress, uint16_t port);
    void onConnectionFailure(const QString& ipAddress, uint16_t port);
    void onProtocolDisconnected();

    // catch signals from listener thread so we have them in the main thread
    void onTrainingStarted(uh::RunningTrainingSession* training);
    void onTrainingEnded();
    void onTrainingReset();
    void onMatchStarted(uh::RunningGameSession* match);
    void onMatchEnded();
    void onPlayerState(
            quint64 frameTimeStamp,
            quint32 frame,
            quint8 playerID,
            float posx,
            float posy,
            float damage,
            float hitstun,
            float shield,
            quint16 status,
            quint64 motion,
            quint8 hit_status,
            quint8 stocks,
            bool attack_connected,
            bool facing_direction);

private:
    void endSessionIfNecessary();

private:
    std::unique_ptr<ProtocolConnectTask> connectTask_;
    std::unique_ptr<ProtocolCommunicateTask> communicateTask_;
    uh::Reference<uh::RunningSession> session_;
};

}
