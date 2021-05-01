#pragma once

#include <QString>
#include <QDir>

namespace uhapp {

class RecordingGroup;
class RecordingManagerListener
{
public:
    virtual void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) = 0;

    virtual void onRecordingManagerGroupAdded(RecordingGroup* group) = 0;
    virtual void onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName) = 0;
    virtual void onRecordingManagerGroupRemoved(RecordingGroup* group) = 0;

    virtual void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) = 0;
    virtual void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onRecordingManagerRecordingSourceRemoved(const QString& name) = 0;

    virtual void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) = 0;
    virtual void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onRecordingManagerVideoSourceRemoved(const QString& name) = 0;
};

}
