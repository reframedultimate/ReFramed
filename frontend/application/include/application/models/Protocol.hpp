#pragma once

#include "application/platform/tcp_socket.h"
#include "application/models/ActiveRecording.hpp"

#include <QVector>
#include <QThread>
#include <QMutex>
#include <QExplicitlySharedDataPointer>

namespace uh {

class MappingInfo;
class Recording;

/*!
 * \brief Decodes the incoming stream from the nintendo switch into structures.
 *
 * When initially connecting, the switch will send tables containing information
 * on which enum values map to what names. The switch only sends this information
 * once, but it is stored by the Protocol class and emitted every time a new
 * match starts.
 *
 * Under normal operation, the sequence of events will be:
 *   1) Single matchStarted() call
 *   2) Multiple newPlayerState() calls
 *   3) Single matchEnded() call
 *
 * If the client prematurely disconnects, matchEnded() will still be emitted
 * before connectionClosed() (if a match is in progress). If no match is in
 * progress, then only connectionClosed() is emitted.
 */
class Protocol : public QThread
{
    Q_OBJECT
public:
    explicit Protocol(tcp_socket socket, QObject* parent=nullptr);
    ~Protocol();

signals:
    // emitted from the listener thread
    void _receiveMatchStarted(ActiveRecording* newRecording);
    void _receiveMatchEnded();
    void _receivePlayerState(
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

private slots:
    // catch signals from listener thread so we have them in the main thread
    void onReceiveMatchStarted(ActiveRecording* newRecording);
    void onReceiveMatchEnded();
    void onReceivePlayerState(
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

signals:
    void recordingStarted(ActiveRecording* recording);
    void recordingEnded(ActiveRecording* recording);
    void serverClosedConnection();

private:
    void run() override;

private:
    tcp_socket socket_;
    QMutex mutex_;
    QExplicitlySharedDataPointer<ActiveRecording> recording_;
    bool requestShutdown_ = false;
};

}
