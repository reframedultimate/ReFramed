#pragma once

#include "rfcommon/tcp_socket.h"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/FighterState.hpp"
#include <memory>
#include <QObject>

namespace rfcommon {
    class MappingInfo;
    class ProtocolListener;
    class RunningSession;
    class RunningGameSession;
    class RunningTrainingSession;
}

namespace rfapp {

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

    rfcommon::RunningSession* runningSession()
        { return session_; }

    rfcommon::ListenerDispatcher<rfcommon::ProtocolListener> dispatcher;

private slots:
    void onConnectionSuccess(void* socket_handle, const QString& ipAddress, quint16 port);
    void onConnectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port);
    void onProtocolDisconnected();

    // catch signals from listener thread so we have them in the main thread
    void onTrainingStartedProxy(rfcommon::RunningTrainingSession* training);
    void onTrainingStartedActually(rfcommon::RunningTrainingSession* training);
    void onTrainingResumed(rfcommon::RunningTrainingSession* training);
    void onTrainingEndedProxy();
    void onTrainingEndedActually();
    void onMatchStarted(rfcommon::RunningGameSession* match);
    void onMatchResumed(rfcommon::RunningGameSession* match);
    void onMatchEnded();
    void onFighterState(
            quint64 frameTimeStamp,
            quint32 frame,
            quint8 playerIdx,
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
            bool facing_direction,
            bool opponent_in_hitlag);

private:
    void endSessionIfNecessary();

private:
    std::unique_ptr<ProtocolConnectTask> connectTask_;
    std::unique_ptr<ProtocolCommunicateTask> communicateTask_;
    rfcommon::SmallVector<rfcommon::SmallVector<rfcommon::FighterState, 2>, 2> stateBuffer_;
    rfcommon::Reference<rfcommon::RunningSession> session_;
    bool trainingEndedProxyWasCalled_ = false;
};

}
