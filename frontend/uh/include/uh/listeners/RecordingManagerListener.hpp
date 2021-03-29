#pragma once

#include <QString>
#include <QDir>

namespace uh {

class RecordingGroup;
class RecordingManagerListener
{
public:
    virtual void onRecordingManagerDefaultRecordingLocationChanged(const QDir& name) = 0;
    virtual void onRecordingManagerGroupAdded(const RecordingGroup& group) = 0;
    virtual void onRecordingManagerGroupRemoved(const RecordingGroup& group) = 0;
};

}
