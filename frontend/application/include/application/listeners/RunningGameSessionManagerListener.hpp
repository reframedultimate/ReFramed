#pragma once

#include "uh/Types.hpp"

class QString;
class QFileInfo;

namespace uh {
    class RunningGameSession;
    class PlayerState;
    class SetFormat;
}

namespace uhapp {

class RunningGameSessionManagerListener
{
public:
    virtual void onRunningGameSessionManagerRecordingStarted(uh::RunningGameSession* recording) = 0;
    virtual void onRunningGameSessionManagerRecordingEnded(uh::RunningGameSession* recording) = 0;

    // We re-propagate all SessionListener events because RunningGameSessionManager
    // allows you to change these properties even when there is no running session
    virtual void onRunningGameSessionManagerP1NameChanged(const QString& name) = 0;
    virtual void onRunningGameSessionManagerP2NameChanged(const QString& name) = 0;
    virtual void onRunningGameSessionManagerSetNumberChanged(uh::SetNumber number) = 0;
    virtual void onRunningGameSessionManagerGameNumberChanged(uh::GameNumber number) = 0;
    virtual void onRunningGameSessionManagerFormatChanged(const uh::SetFormat& format) = 0;
    virtual void onRunningGameSessionManagerPlayerStateAdded(int player, const uh::PlayerState& state) = 0;
    virtual void onRunningGameSessionManagerWinnerChanged(int winner) = 0;
};

}

