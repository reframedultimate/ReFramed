#include "uh/models/RecordingManager.hpp"
#include "uh/models/Settings.hpp"
#include "uh/listeners/RecordingManagerListener.hpp"

#include <QStandardPaths>

namespace uh {

// ----------------------------------------------------------------------------
RecordingManager::RecordingManager(Settings* settings)
    : settings_(settings)
{
    // Default location is always at index 0
    recordingDirectories_.push_back(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    recordingGroups_.insert({"All", std::make_unique<RecordingGroup>("All")});
}

// ----------------------------------------------------------------------------
const QDir& RecordingManager::defaultRecordingSourceDirectory() const
{
    return recordingDirectories_[0];
}

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
RecordingGroup* RecordingManager::allRecordingGroup()
{
    return recordingGroups_["All"].get();
}

// ----------------------------------------------------------------------------
const std::unordered_map<QString, std::unique_ptr<RecordingGroup>>& RecordingManager::recordingGroups() const
{
    return recordingGroups_;
}

// ----------------------------------------------------------------------------
RecordingGroup* RecordingManager::recordingGroup(const QString& name)
{
    auto it = recordingGroups_.find(name);
    if (it == recordingGroups_.end())
        return nullptr;
    return it->second.get();
}

// ----------------------------------------------------------------------------
RecordingGroup* RecordingManager::getOrCreateRecordingGroup(const QString& name)
{
    RecordingGroup* group = recordingGroup(name);
    if (group)
        return group;

    auto it = recordingGroups_.insert({name, std::make_unique<RecordingGroup>(name)});
    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerGroupAdded, it.first->second.get());
    return it.first->second.get();
}

// ----------------------------------------------------------------------------
void RecordingManager::rescanForRecordings()
{
    RecordingGroup* allGroup = allRecordingGroup();
    allGroup->removeAllFiles();
    for (const auto& recdir : recordingDirectories_)
        for (const auto& file : recdir.entryList({"*.uhr", "*.UHR"}, QDir::Files))
            allGroup->addFile(recdir.absoluteFilePath(file));
}

// ----------------------------------------------------------------------------
void RecordingManager::setDefaultRecordingSourceDirectory(const QDir& path)
{
    recordingDirectories_[0] = path;
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
