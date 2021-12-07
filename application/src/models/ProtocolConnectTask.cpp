#include "application/models/ProtocolConnectTask.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
ProtocolConnectTask::ProtocolConnectTask(const QString& ipAddress, uint16_t port, QObject* parent)
    : QThread(parent)
    , ipAddress_(ipAddress)
    , port_(port)
{}

// ----------------------------------------------------------------------------
ProtocolConnectTask::~ProtocolConnectTask()
{
    wait();
}

// ----------------------------------------------------------------------------
void ProtocolConnectTask::run()
{
    tcp_socket socket;
    QByteArray ba = ipAddress_.toLocal8Bit();
    if (tcp_socket_connect_to_host(&socket, ba.data(), port_) != 0)
    {
        emit connectionFailure(
                    tcp_socket_get_last_error(&socket),
                    ipAddress_, port_);
        return;
    }

    // TODO: Get protocol version from switch and verify that this is even
    // a switch.

    emit connectionSuccess(socket, ipAddress_, port_);
}

}
