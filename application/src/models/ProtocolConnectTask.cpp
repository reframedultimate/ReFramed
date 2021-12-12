#include "application/models/ProtocolConnectTask.hpp"
#include "application/models/ProtocolCommunicateTask.hpp"
#include "rfcommon/tcp_socket.h"

namespace rfapp {

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

    // Get protocol version from switch and verify we're compatible
    char buf[3] = {ProtocolCommunicateTask::ProtocolVersion};
    if (tcp_socket_write(&socket, buf, 1) != 1)
        goto socket_error;

    while (true)
    {
        if (tcp_socket_read(&socket, buf, 1) != 1)
            goto socket_error;

        switch (buf[0])
        {
            case ProtocolCommunicateTask::ProtocolVersion: {
                if (tcp_socket_read(&socket, buf, 2) != 2)
                    goto socket_error;

                const char major = 0x01;
                const char minor = 0x00;
                if (buf[0] == major && buf[1] == minor)
                    break;

                emit connectionFailure(
                            QString("Unsupported protocol version major=%1, minor=%2").arg(buf[0]).arg(buf[1]),
                            ipAddress_, port_);
                tcp_socket_close(&socket);
                return;

            } break;

            case ProtocolCommunicateTask::MappingInfoChecksum:
            case ProtocolCommunicateTask::MappingInfoRequest:
            case ProtocolCommunicateTask::MappingInfoFighterKinds:
            case ProtocolCommunicateTask::MappingInfoFighterStatusKinds:
            case ProtocolCommunicateTask::MappingInfoStageKinds:
            case ProtocolCommunicateTask::MappingInfoHitStatusKinds:
            case ProtocolCommunicateTask::MappingInfoRequestComplete:
            case ProtocolCommunicateTask::MatchStart:
            case ProtocolCommunicateTask::MatchEnd:
            case ProtocolCommunicateTask::TrainingStart:
            case ProtocolCommunicateTask::TrainingReset:
            case ProtocolCommunicateTask::TrainingEnd:
            case ProtocolCommunicateTask::FighterState:
                continue;
        }

        break;
    }

    emit connectionSuccess(tcp_socket_to_handle(&socket), ipAddress_, port_);
    return;

socket_error:
    emit connectionFailure(
                tcp_socket_get_last_error(&socket),
                ipAddress_, port_);
    tcp_socket_close(&socket);
}

}
