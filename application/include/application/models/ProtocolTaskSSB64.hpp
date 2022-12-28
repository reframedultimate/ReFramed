#pragma once

#include <QThread>
#include <QMutex>

namespace rfcommon {
    class Log;
    class MappingInfo;
    class Metadata;
}

namespace rfapp {

class ProtocolTaskSSB64 : public QThread
{
    Q_OBJECT

public:
    ProtocolTaskSSB64(rfcommon::Log* log, QObject* parent=nullptr);
    ~ProtocolTaskSSB64();

signals:
    void connectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port);
    void connectionSuccess(const QString& ipAddress, quint16 port);
    void connectionClosed();

    void trainingStarted(rfcommon::Metadata* training);
    void trainingResumed(rfcommon::Metadata* training);
    void trainingEnded();
    void gameStarted(rfcommon::Metadata* game);
    void gameResumed(rfcommon::Metadata* game);
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
    rfcommon::Log* log_;
    QMutex mutex_;
    bool requestShutdown_;
};

}
