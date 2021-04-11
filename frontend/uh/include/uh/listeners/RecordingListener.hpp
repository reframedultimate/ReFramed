#pragma once

#include <QString>
#include "uh/models/SetFormat.hpp"

namespace uh {

class PlayerState;

class RecordingListener
{
public:
    virtual void onActiveRecordingPlayerNameChanged(int player, const QString& name) = 0;
    virtual void onActiveRecordingSetNumberChanged(int number) = 0;
    virtual void onActiveRecordingGameNumberChanged(int number) = 0;
    virtual void onActiveRecordingFormatChanged(const SetFormat& format) = 0;
    virtual void onActiveRecordingPlayerStateAdded(int player, const PlayerState& state) = 0;
};

}
