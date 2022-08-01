#pragma once

#include <QThread>
#include <QMutex>

namespace rfcommon {
    class MappingInfo;
    class SessionMetaData;
}

namespace rfapp {

class ProtocolTask : public QThread
{
    Q_OBJECT

public:
    enum MessageType
    {
        ProtocolVersion,

        MappingInfoChecksum,
        MappingInfoRequest,
        MappingInfoFighterKinds,
        MappingInfoFighterStatusKinds,
        MappingInfoStageKinds,
        MappingInfoHitStatusKinds,
        MappingInfoRequestComplete,

        GameStart,
        GameResume,
        GameEnd,
        TrainingStart,
        TrainingResume,
        TrainingReset,
        TrainingEnd,

        FighterState,
    };

    ProtocolTask(const QString& ipAddress, quint16 port, uint32_t mappingInfoChecksum, QObject* parent=nullptr);
    ~ProtocolTask();

signals:
    void connectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port);
    void connectionSuccess(const QString& ipAddress, quint16 port);
    void connectionClosed();

    void mappingInfoReceived(rfcommon::MappingInfo* mappingInfo);
    void trainingStarted(rfcommon::SessionMetaData* training);
    void trainingResumed(rfcommon::SessionMetaData* training);
    void trainingEnded();
    void gameStarted(rfcommon::SessionMetaData* game);
    void gameResumed(rfcommon::SessionMetaData* game);
    void gameEnded();
    void fighterState(
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
    void run() override;
    void* connectAndCheckVersion();
    bool negotiateMappingInfo(void* tcp_socket_handle);
    void handleProtocol(void* tcp_socket_handle);

private:
    QString ipAddress_;
    QMutex mutex_;
    uint32_t mappingInfoChecksum_;
    uint16_t port_;
    bool requestShutdown_;
};

}