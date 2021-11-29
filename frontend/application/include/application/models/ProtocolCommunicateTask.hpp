#pragma once

#include "uh/tcp_socket.h"
#include <QThread>
#include <QMutex>

namespace uh {
    class RunningTrainingSession;
    class RunningGameSession;
}

namespace uhapp {

class ProtocolCommunicateTask : public QThread
{
    Q_OBJECT

    enum MessageType
    {
        Version,
        TrainingStart,
        TrainingEnd,
        TrainingReset,
        MatchStart,
        MatchEnd,
        FighterState,
        FighterKinds,
        FighterStatusKinds,
        StageKinds,
        HitStatusKinds
    };

public:
    ProtocolCommunicateTask(tcp_socket socket, QObject* parent=nullptr);
    ~ProtocolCommunicateTask();

signals:
    void connectionClosed();
    void trainingStarted(uh::RunningTrainingSession* training);
    void trainingEnded();
    void trainingReset();
    void matchStarted(uh::RunningGameSession* match);
    void matchEnded();
    void playerState(
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
    void run() override;

private:
    tcp_socket socket_;
    QMutex mutex_;
    bool requestShutdown_;
};

}
