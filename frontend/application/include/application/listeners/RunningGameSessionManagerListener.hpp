#pragma once

#include "uh/Types.hpp"
#include "uh/String.hpp"

namespace uh {
    class RunningGameSession;
    class PlayerState;
    class SetFormat;
}

namespace uhapp {

class RunningGameSessionManagerListener
{
public:
    // We re-propagate all RunningGameSession SessionListener events because
    // RunningGameSessionManager allows you to change these properties even when there is no running session
    virtual void onRunningGameSessionManagerNewPlayerState(int player, const uh::PlayerState& state) = 0;
    virtual void onRunningGameSessionManagerPlayerNameChanged(int player, const uh::SmallString<15>& name) = 0;
    virtual void onRunningGameSessionManagerSetNumberChanged(uh::SetNumber number) = 0;
    virtual void onRunningGameSessionManagerGameNumberChanged(uh::GameNumber number) = 0;
    virtual void onRunningGameSessionManagerFormatChanged(const uh::SetFormat& format) = 0;
    virtual void onRunningGameSessionManagerWinnerChanged(int winner) = 0;

    // We also re-propagate all (to RunningGameSessionManager) relevant ProtocolListener
    // events because it makes interfacing with RunningGameSessionManager less complicated.
    virtual void onRunningGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onRunningGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onRunningGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onRunningGameSessionManagerDisconnectedFromServer() = 0;

    virtual void onRunningGameSessionManagerMatchStarted(uh::RunningGameSession* recording) = 0;
    virtual void onRunningGameSessionManagerMatchEnded(uh::RunningGameSession* recording) = 0;
};

}
