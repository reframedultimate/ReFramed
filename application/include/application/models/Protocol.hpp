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
    class Metadata;
    class Session;
}

namespace rfapp {

class ProtocolTask;
class ProtocolTaskSSB64;

/*!
 * \brief Decodes the incoming stream from the Nintendo switch into structures.
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
    void connectToSSB64Process();
    void disconnectFromServer();

    rfcommon::MappingInfo* globalMappingInfo() const;

    rfcommon::ListenerDispatcher<rfcommon::ProtocolListener> dispatcher;

private slots:
    // catch signals from listener thread so we have them in the main thread
    void onConnectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port);
    void onConnectionSuccess(const QString& ipAddress, quint16 port);
    void onConnectionClosed();

    void onMappingInfoReceived(rfcommon::MappingInfo* mappingInfo);
    void onTrainingStartedProxy(rfcommon::Metadata* trainingMeta);
    void onTrainingStartedActually(rfcommon::Metadata* trainingMeta);
    void onTrainingResumed(rfcommon::Metadata* trainingMeta);
    void onTrainingEndedProxy();
    void onTrainingEndedActually();
    void onGameStarted(rfcommon::Metadata* gameMeta);
    void onGameResumed(rfcommon::Metadata* gameMeta);
    void onGameEnded();
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
            bool attackConnected,
            bool facingDirection,
            bool opponentInHitlag);

private:
    void endSessionIfNecessary();
    void tryLoadGlobalMappingInfo();
    void saveGlobalMappingInfo();

private:
    rfcommon::Reference<rfcommon::MappingInfo> globalMappingInfo_;
    std::unique_ptr<ProtocolTask> task_;
    std::unique_ptr<ProtocolTaskSSB64> ssb64Task_;
    rfcommon::SmallVector<rfcommon::SmallVector<rfcommon::FighterState, 2>, 2> stateBuffer_;
    rfcommon::Reference<rfcommon::Session> activeSession_;
    bool trainingEndedProxyWasCalled_;
};

}
