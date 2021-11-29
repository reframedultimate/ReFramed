#pragma once

#include "uh/tcp_socket.h"
#include <QThread>
#include <QString>

namespace uhapp {

class ProtocolConnectTask : public QThread
{
    Q_OBJECT

public:
    ProtocolConnectTask(const QString& ipAddress, uint16_t port, QObject* parent=nullptr);
    ~ProtocolConnectTask();

signals:
    void connectionSuccess(tcp_socket socket, const QString& ipAddress, uint16_t port);
    void connectionFailure(const QString& ipAddress, uint16_t port);

private:
    void run() override;

private:
    QString ipAddress_;
    uint16_t port_;
};

}
