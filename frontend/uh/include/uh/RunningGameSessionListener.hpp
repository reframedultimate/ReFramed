#pragma once

#include "uh/config.hpp"
#include "uh/SetFormat.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"

namespace uh {

class PlayerState;

class RunningGameSessionListener
{
public:
    virtual void onRunningGameSessionPlayerNameChanged(int player, const SmallString<15>& name) = 0;
    virtual void onRunningGameSessionSetNumberChanged(SetNumber number) = 0;
    virtual void onRunningGameSessionGameNumberChanged(GameNumber number) = 0;
    virtual void onRunningGameSessionFormatChanged(const SetFormat& format) = 0;
    virtual void onRunningGameSessionNewUniquePlayerState(int player, const PlayerState& state) = 0;
    virtual void onRunningGameSessionNewPlayerState(int player, const PlayerState& state) = 0;

    virtual void onRecordingWinnerChanged(int winner) = 0;
};

}
