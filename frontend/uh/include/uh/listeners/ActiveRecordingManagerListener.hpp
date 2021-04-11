#pragma once

#include "uh/models/SetFormat.hpp"
#include <QString>

namespace uh {

class ActiveRecording;
class PlayerState;

class ActiveRecordingManagerListener
{
public:
    virtual void onActiveRecordingManagerRecordingStarted(ActiveRecording* recording) = 0;
    virtual void onActiveRecordingManagerRecordingEnded(ActiveRecording* recording) = 0;
    virtual void onActiveRecordingManagerRecordingSaved(const QString& fileName) = 0;

    // We re-propagate all RecordingListener events because ActiveRecordingManager
    // allows you to change these properties even when there is no active recording
    virtual void onActiveRecordingManagerP1NameChanged(const QString& name) = 0;
    virtual void onActiveRecordingManagerP2NameChanged(const QString& name) = 0;
    virtual void onActiveRecordingManagerSetNumberChanged(int number) = 0;
    virtual void onActiveRecordingManagerGameNumberChanged(int number) = 0;
    virtual void onActiveRecordingManagerFormatChanged(const SetFormat& format) = 0;
    virtual void onActiveRecordingManagerPlayerStateAdded(int player, const PlayerState& state) = 0;
};

}

