#pragma once

#include "rfcommon/tcp_socket.h"
#include <QThread>
#include <QString>

namespace rfapp {

class ProtocolConnectTask : public QThread
{
    Q_OBJECT

public:
    ProtocolConnectTask(const QString& ipAddress, uint16_t port, QObject* parent=nullptr);
    ~ProtocolConnectTask();

signals:
    void connectionSuccess(tcp_socket socket, const QString& ipAddress, quint16 port);
    void connectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port);

private:
    void run() override;

private:
    QString ipAddress_;
    uint16_t port_;
};

}
