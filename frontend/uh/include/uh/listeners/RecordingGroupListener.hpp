#pragma once

#include <QString>
#include <QDir>

namespace uh {

class RecordingGroup;

class RecordingGroupListener
{
public:
    virtual void onRecordingGroupNameChanged(const QString& name) = 0;
    virtual void onRecordingGroupFileAdded(const QDir& name) = 0;
    virtual void onRecordingGroupFileRemoved(const QDir& name) = 0;
};

}
