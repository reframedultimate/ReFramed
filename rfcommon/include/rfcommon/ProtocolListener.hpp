#pragma once

#include <cstdint>

namespace rfcommon {

class Session;

class ProtocolListener
{
public:
    virtual void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onProtocolDisconnectedFromServer() = 0;

    virtual void onProtocolTrainingStarted(rfcommon::Session* training) = 0;
    virtual void onProtocolTrainingResumed(rfcommon::Session* training) = 0;
    virtual void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) = 0;
    virtual void onProtocolTrainingEnded(rfcommon::Session* training) = 0;
    virtual void onProtocolGameStarted(rfcommon::Session* game) = 0;
    virtual void onProtocolGameResumed(rfcommon::Session* game) = 0;
    virtual void onProtocolGameEnded(rfcommon::Session* game) = 0;
};

}
