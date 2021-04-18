#pragma once

#include <QString>
#include <QFileInfo>

namespace uh {

class RecordingGroup;

class RecordingGroupListener
{
public:
    virtual void onRecordingGroupNameChanged(const QString& name) = 0;
    virtual void onRecordingGroupFileAdded(const QFileInfo& absPathToFile) = 0;
    virtual void onRecordingGroupFileRemoved(const QFileInfo& absPathToFile) = 0;
};

}
