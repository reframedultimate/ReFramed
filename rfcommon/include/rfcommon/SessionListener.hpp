#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {

class PlayerState;

class SessionListener
{
public:
    // RunningGameSession events
    virtual void onRunningGameSessionPlayerNameChanged(int playerIdx, const SmallString<15>& name) = 0;
    virtual void onRunningGameSessionSetNumberChanged(SetNumber number) = 0;
    virtual void onRunningGameSessionGameNumberChanged(GameNumber number) = 0;
    virtual void onRunningGameSessionFormatChanged(const SetFormat& format) = 0;
    virtual void onRunningGameSessionWinnerChanged(int winnerPlayerIdx) = 0;

    // RunningSession events
    virtual void onRunningSessionNewUniquePlayerState(int playerIdx, const PlayerState& state) = 0;
    virtual void onRunningSessionNewPlayerState(int playerIdx, const PlayerState& state) = 0;
    virtual void onRunningSessionNewUniqueFrame(const SmallVector<PlayerState, 8>& states) = 0;
    virtual void onRunningSessionNewFrame(const SmallVector<PlayerState, 8>& states) = 0;
};

}
