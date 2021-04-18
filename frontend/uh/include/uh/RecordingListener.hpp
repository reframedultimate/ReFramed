#pragma once

#include "uh/SetFormat.hpp"
#include <string>

namespace uh {

class PlayerState;

class RecordingListener
{
public:
    virtual void onActiveRecordingPlayerNameChanged(int player, const std::wstring& name) = 0;
    virtual void onActiveRecordingSetNumberChanged(int number) = 0;
    virtual void onActiveRecordingGameNumberChanged(int number) = 0;
    virtual void onActiveRecordingFormatChanged(const SetFormat& format) = 0;
    virtual void onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state) = 0;
    virtual void onActiveRecordingNewPlayerState(int player, const PlayerState& state) = 0;

    virtual void onRecordingWinnerChanged(int winner) = 0;
};

}
