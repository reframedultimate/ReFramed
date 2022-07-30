#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"

namespace rfcommon {
    class Frame;
    class Session;
    class SetFormat;
}

namespace rfapp {

class ActiveGameSessionManagerListener
{
public:
    // We re-propagate all Session frame and meta-data events because
    // ActiveGameSessionManager allows you to change these properties even when
    // there is no active session
    virtual void onActiveGameSessionManagerNewFrame(int frameIdx, const rfcommon::Frame& frame) = 0;
    virtual void onActiveGameSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) = 0;
    virtual void onActiveGameSessionManagerSetNumberChanged(rfcommon::SetNumber number) = 0;
    virtual void onActiveGameSessionManagerGameNumberChanged(rfcommon::GameNumber number) = 0;
    virtual void onActiveGameSessionManagerFormatChanged(const rfcommon::SetFormat& format) = 0;
    virtual void onActiveGameSessionManagerWinnerChanged(int winner) = 0;

    // We also re-propagate all (to RunningGameSessionManager) relevant ProtocolListener
    // events because it makes interfacing with RunningGameSessionManager less complicated.
    // TODO: This is stupid, remove all of these
    virtual void onActiveGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onActiveGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onActiveGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) = 0;
    virtual void onActiveGameSessionManagerDisconnectedFromServer() = 0;

    virtual void onActiveGameSessionManagerGameStarted(rfcommon::Session* session) = 0;
    virtual void onActiveGameSessionManagerGameEnded(rfcommon::Session* session) = 0;
};

}
