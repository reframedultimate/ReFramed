#pragma once

#include <QString>
#include <QDir>

namespace uh {

class RecordingGroup;
class RecordingManagerListener
{
public:
    virtual void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) = 0;
    virtual void onRecordingManagerGroupAdded(RecordingGroup* group) = 0;
    virtual void onRecordingManagerGroupRemoved(RecordingGroup* group) = 0;
};

}
