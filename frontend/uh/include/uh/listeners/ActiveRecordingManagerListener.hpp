#pragma once

#include "uh/models/SetFormat.hpp"
#include <QString>

namespace uh {

class ActiveRecording;
class PlayerState;

class ActiveRecordingManagerListener
{
public:
    virtual void onActiveRecordingManagerRecordingStarted(ActiveRecording* recording)  { (void)recording; }
    virtual void onActiveRecordingManagerRecordingEnded(ActiveRecording* recording)  { (void)recording; }
    virtual void onActiveRecordingManagerRecordingSaved(const QString& fileName) { (void)fileName; }

    virtual void onActiveRecordingManagerP1NameChanged(const QString& name) { (void)name; }
    virtual void onActiveRecordingManagerP2NameChanged(const QString& name) { (void)name; }
    virtual void onActiveRecordingManagerFormatChanged(SetFormat format, const QString& otherFormatDesc)  { (void)format; (void)otherFormatDesc; }
    virtual void onActiveRecordingManagerSetNumberChanged(int number) { (void)number; }
    virtual void onActiveRecordingManagerGameNumberChanged(int number) { (void)number; }

    virtual void onActiveRecordingManagerPlayerStateAdded(int player, const PlayerState& state) { (void)player; (void)state; }
};

}

