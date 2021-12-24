#pragma once

#include "rfcommon/tcp_socket.h"
#include <QThread>
#include <QMutex>
#include <QString>

namespace rfapp {

class ProtocolConnectTask : public QThread
{
    Q_OBJECT

public:
    ProtocolConnectTask(const QString& ipAddress, uint16_t port, QObject* parent=nullptr);
    ~ProtocolConnectTask();

signals:
    void connectionSuccess(void* socket_handle, const QString& ipAddress, quint16 port);
    void connectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port);

private:
    void run() override;

private:
    tcp_socket socket_;
    QString ipAddress_;
    uint16_t port_;
    QMutex mutex_;
    bool socketCreated_;
};

}
