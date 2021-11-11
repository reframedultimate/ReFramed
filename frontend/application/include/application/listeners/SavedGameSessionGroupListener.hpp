#pragma once

#include <QString>
#include <QFileInfo>

namespace uhapp {

class RecordingGroup;

class RecordingGroupListener
{
public:
    virtual void onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& absPathToFile) = 0;
    virtual void onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& absPathToFile) = 0;
};

}
