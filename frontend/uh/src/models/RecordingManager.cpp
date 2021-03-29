#include "uh/models/RecordingManager.hpp"
#include "uh/listeners/RecordingManagerListener.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const QVector<QDir>& RecordingManager::recordingSourceDirectories() const
{
    return recordingDirectories_;
}

// ----------------------------------------------------------------------------
const QVector<QDir>& RecordingManager::videoSourceDirectories()
{
    return videoDirectories_;
}

// ----------------------------------------------------------------------------
const QVector<RecordingGroup>& RecordingManager::recordingGroups() const
{
    return recordingGroups_;
}

// ----------------------------------------------------------------------------
RecordingGroup* RecordingManager::recordingGroup(const QString& name)
{
    for (auto& group : recordingGroups_)
        if (group.name() == name)
            return &group;
    return nullptr;
}

// ----------------------------------------------------------------------------
RecordingGroup* RecordingManager::getOrCreateRecordingGroup(const QString& name)
{
    RecordingGroup* group = recordingGroup(name);
    if (group)
        return group;

    recordingGroups_.push_back(RecordingGroup(name));
    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerGroupAdded, recordingGroups_.back());
    return &recordingGroups_.back();
}

// ----------------------------------------------------------------------------
void RecordingManager::rescanForRecordings()
{

}

// ----------------------------------------------------------------------------
void RecordingManager::setDefaultRecordingLocation(const QDir& path)
{
    defaultRecordingLocation_ = path;
    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerDefaultRecordingLocationChanged, path);
}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupNameChanged(const QString& name)
{

}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupFileAdded(const QDir& name)
{

}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupFileRemoved(const QDir& name)
{

}

}
