#pragma once

#include <cstdint>

namespace rfcommon {

class RunningTrainingSession;
class RunningGameSession;

class ProtocolListener
{
public:
    virtual void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolDisconnectedFromServer() = 0;

    virtual void onProtocolTrainingStarted(rfcommon::RunningTrainingSession* session) = 0;
    virtual void onProtocolTrainingResumed(rfcommon::RunningTrainingSession* session) = 0;
    virtual void onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldSession, rfcommon::RunningTrainingSession* newSession) = 0;
    virtual void onProtocolTrainingEnded(rfcommon::RunningTrainingSession* session) = 0;
    virtual void onProtocolMatchStarted(rfcommon::RunningGameSession* session) = 0;
    virtual void onProtocolMatchResumed(rfcommon::RunningGameSession* session) = 0;
    virtual void onProtocolMatchEnded(rfcommon::RunningGameSession* session) = 0;
};

}
