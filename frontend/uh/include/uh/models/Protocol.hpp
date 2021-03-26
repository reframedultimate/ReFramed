#pragma once

#include "uh/platform/tcp_socket.h"
#include "uh/models/Recording.hpp"

#include <QVector>
#include <QThread>
#include <QMutex>
#include <QSharedDataPointer>

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
    void dateChanged(const QDateTime& date);
    void stageChanged(const QString& name);
    void playerCountChanged(int count);
    void playerTagChanged(int index, const QString& tag);
    void playerFighterChanged(int index, const QString& fighterName);

    void matchStarted();
    void playerStatusChanged(unsigned int frame, int index, unsigned int status);
    void playerDamageChanged(unsigned int frame, int index, float damage);
    void playerStockCountChanged(unsigned int frame, int index, unsigned char stocks);
    void matchEnded(Recording* recording);

    void connectionClosed();

private:
    void run() override;

private:
    tcp_socket socket_;
    QScopedPointer<MappingInfo> mapping_;
    QSharedDataPointer<Recording> recording_;
    QMutex mutex_;
    bool requestShutdown_ = false;
};

}
