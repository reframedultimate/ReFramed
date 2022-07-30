#pragma once

#include "rfcommon/tcp_socket.h"
#include <QThread>
#include <QMutex>

namespace rfcommon {
    class Session;
}

namespace rfapp {

class ProtocolCommunicateTask : public QThread
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

        MatchStart,
        MatchResume,
        MatchEnd,
        TrainingStart,
        TrainingResume,
        TrainingReset,
        TrainingEnd,

        FighterState,
    };

    ProtocolCommunicateTask(tcp_socket socket, QObject* parent=nullptr);
    ~ProtocolCommunicateTask();

signals:
    void connectionClosed();
    void trainingStarted(rfcommon::Session* training);
    void trainingResumed(rfcommon::Session* training);
    void trainingEnded();
    void gameStarted(rfcommon::Session* game);
    void gameResumed(rfcommon::Session* game);
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

private:
    tcp_socket socket_;
    QMutex mutex_;
    bool requestShutdown_;
};

}
