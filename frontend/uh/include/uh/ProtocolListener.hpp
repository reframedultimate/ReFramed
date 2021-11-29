#pragma once

#include <cstdint>

namespace uh {

class RunningTrainingSession;
class RunningGameSession;

class ProtocolListener
{
public:
    virtual void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolFailedToConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolDisconnectedFromServer() = 0;

    virtual void onProtocolTrainingStarted(uh::RunningTrainingSession* session) = 0;
    virtual void onProtocolTrainingEnded(uh::RunningTrainingSession* session) = 0;
    virtual void onProtocolMatchStarted(uh::RunningGameSession* session) = 0;
    virtual void onProtocolMatchEnded(uh::RunningGameSession* session) = 0;
};

}
