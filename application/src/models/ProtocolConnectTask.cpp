#include "application/models/ProtocolConnectTask.hpp"
#include "application/models/ProtocolCommunicateTask.hpp"
#include "rfcommon/tcp_socket.h"

namespace rfapp {

// ----------------------------------------------------------------------------
ProtocolConnectTask::ProtocolConnectTask(const QString& ipAddress, uint16_t port, QObject* parent)
    : QThread(parent)
    , ipAddress_(ipAddress)
    , port_(port)
    , socketCreated_(false)
{
}

// ----------------------------------------------------------------------------
ProtocolConnectTask::~ProtocolConnectTask()
{
    mutex_.lock();
        if (socketCreated_)
            tcp_socket_shutdown(&socket_);
    mutex_.unlock();

    wait();

    if (socketCreated_)
        tcp_socket_shutdown(&socket_);
}

// ----------------------------------------------------------------------------
void ProtocolConnectTask::run()
{
    QByteArray ba = ipAddress_.toLocal8Bit();
    mutex_.lock();
        if (tcp_socket_connect_to_host(&socket_, ba.data(), port_) != 0)
        {
            emit connectionFailure(
                        tcp_socket_get_connect_error(&socket_),
                        ipAddress_, port_);
            mutex_.unlock();
            return;
        }

        socketCreated_ = true;
    mutex_.unlock();

    // Get protocol version from switch and verify we're compatible
    char buf[3] = {ProtocolCommunicateTask::ProtocolVersion};
    const char* errormsg = "";
    if (tcp_socket_write(&socket_, buf, 1) != 1)
    {
        errormsg = "Write error";
        goto socket_error;
    }

    while (true)
    {
        if (tcp_socket_read(&socket_, buf, 1) != 1)
        {
            errormsg = "Read error";
            goto socket_error;
        }

        switch (buf[0])
        {
            case ProtocolCommunicateTask::ProtocolVersion: {
                if (tcp_socket_read(&socket_, buf, 2) != 2)
                {
                    errormsg = "Read error";
                    goto socket_error;
                }

                const char major = 0x01;
                const char minor = 0x00;
                if (buf[0] == major && buf[1] == minor)
                    break;

                emit connectionFailure(
                            QString("Unsupported protocol version major=%1, minor=%2").arg(buf[0]).arg(buf[1]),
                            ipAddress_, port_);
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

    mutex_.lock();
        socketCreated_ = false;
    mutex_.unlock();

    emit connectionSuccess(tcp_socket_to_handle(&socket_), ipAddress_, port_);
    return;

socket_error:
    emit connectionFailure(errormsg, ipAddress_, port_);
}

}
