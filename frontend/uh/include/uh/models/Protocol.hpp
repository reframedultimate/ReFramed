#pragma once

#include "uh/platform/tcp_socket.h"
#include "uh/models/ActiveRecording.hpp"

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
    void _receivePlayerState(unsigned int frame, int playerID, unsigned int status, float damage, unsigned int stocks);
    void _receiveMatchEnded();

private slots:
    // catch signals from listener thread so we have them in the main thread
    void onReceiveMatchStarted(ActiveRecording* newRecording);
    void onReceivePlayerState(unsigned int frame, int playerID, unsigned int status, float damage, unsigned int stocks);
    void onReceiveMatchEnded();

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
