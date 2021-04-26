#pragma once

#include "uh/SetFormat.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"

namespace uh {

class PlayerState;

class RecordingListener
{
public:
    virtual void onActiveRecordingPlayerNameChanged(int player, const SmallString<15>& name) = 0;
    virtual void onActiveRecordingSetNumberChanged(SetNumber number) = 0;
    virtual void onActiveRecordingGameNumberChanged(GameNumber number) = 0;
    virtual void onActiveRecordingFormatChanged(const SetFormat& format) = 0;
    virtual void onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state) = 0;
    virtual void onActiveRecordingNewPlayerState(int player, const PlayerState& state) = 0;

    virtual void onRecordingWinnerChanged(int winner) = 0;
};

}
