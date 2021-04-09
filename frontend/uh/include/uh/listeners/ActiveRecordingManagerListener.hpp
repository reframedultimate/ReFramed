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

    virtual void onActiveRecordingManagerP1NameChanged(const QString& name) = 0;
    virtual void onActiveRecordingManagerP2NameChanged(const QString& name) = 0;
    virtual void onActiveRecordingManagerFormatChanged(SetFormat format, const QString& otherFormatDesc) = 0;
    virtual void onActiveRecordingManagerSetNumberChanged(int number) = 0;
    virtual void onActiveRecordingManagerGameNumberChanged(int number) = 0;

    virtual void onActiveRecordingManagerPlayerStateAdded(int player, const PlayerState& state) = 0;
};

}

