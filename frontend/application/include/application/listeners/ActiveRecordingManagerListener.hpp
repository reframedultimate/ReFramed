#pragma once

#include "uh/Types.hpp"

class QString;
class QFileInfo;

namespace uh {
    class ActiveRecording;
    class PlayerState;
    class SetFormat;
}

namespace uhapp {

class ActiveRecordingManagerListener
{
public:
    virtual void onActiveRecordingManagerRecordingStarted(uh::ActiveRecording* recording) = 0;
    virtual void onActiveRecordingManagerRecordingEnded(uh::ActiveRecording* recording) = 0;

    // We re-propagate all RecordingListener events because ActiveRecordingManager
    // allows you to change these properties even when there is no active recording
    virtual void onActiveRecordingManagerP1NameChanged(const QString& name) = 0;
    virtual void onActiveRecordingManagerP2NameChanged(const QString& name) = 0;
    virtual void onActiveRecordingManagerSetNumberChanged(uh::SetNumber number) = 0;
    virtual void onActiveRecordingManagerGameNumberChanged(uh::GameNumber number) = 0;
    virtual void onActiveRecordingManagerFormatChanged(const uh::SetFormat& format) = 0;
    virtual void onActiveRecordingManagerPlayerStateAdded(int player, const uh::PlayerState& state) = 0;
    virtual void onActiveRecordingManagerWinnerChanged(int winner) = 0;
};

}

