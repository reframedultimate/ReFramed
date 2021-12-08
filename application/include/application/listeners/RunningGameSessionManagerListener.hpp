#pragma once

#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {
    class RunningGameSession;
    class PlayerState;
    class SetFormat;
}

namespace rfapp {

class RunningGameSessionManagerListener
{
public:
    // We re-propagate all RunningGameSession SessionListener events because
    // RunningGameSessionManager allows you to change these properties even when there is no running session
    virtual void onRunningGameSessionManagerNewPlayerState(int player, const rfcommon::PlayerState& state) = 0;
    virtual void onRunningGameSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) = 0;
    virtual void onRunningGameSessionManagerSetNumberChanged(rfcommon::SetNumber number) = 0;
    virtual void onRunningGameSessionManagerGameNumberChanged(rfcommon::GameNumber number) = 0;
    virtual void onRunningGameSessionManagerFormatChanged(const rfcommon::SetFormat& format) = 0;
    virtual void onRunningGameSessionManagerWinnerChanged(int winner) = 0;

    // We also re-propagate all (to RunningGameSessionManager) relevant ProtocolListener
    // events because it makes interfacing with RunningGameSessionManager less complicated.
    virtual void onRunningGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onRunningGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onRunningGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onRunningGameSessionManagerDisconnectedFromServer() = 0;

    virtual void onRunningGameSessionManagerMatchStarted(rfcommon::RunningGameSession* recording) = 0;
    virtual void onRunningGameSessionManagerMatchEnded(rfcommon::RunningGameSession* recording) = 0;
};

}
