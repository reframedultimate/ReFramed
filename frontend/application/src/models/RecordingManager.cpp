#include "application/Util.hpp"
#include "application/models/RecordingManager.hpp"
#include "application/listeners/RecordingManagerListener.hpp"

#include <QStandardPaths>
#include <QJsonObject>

namespace uhapp {

// ----------------------------------------------------------------------------
RecordingManager::RecordingManager(Config* config)
    : ConfigAccessor(config)
{
    QJsonObject& cfg = getConfig();
    if (cfg["recordingmanager"].isNull())
        cfg["recordingmanager"] = QJsonObject();
    auto cfgRecMgr = cfg["recordingmanager"].toObject();
    if (cfgRecMgr["defaultrecordingdir"].isNull())
        cfgRecMgr["defaultrecordingdir"] = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).canonicalPath();
    cfg["recordingmanager"] = cfgRecMgr;

    // Default location is always at index 0
    recordingSources_.insert("Default", defaultRecordingSourceDirectory());

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    recordingGroups_.insert({"All", std::make_unique<RecordingGroup>("All")});

    scanForRecordings();
}

// ----------------------------------------------------------------------------
QDir RecordingManager::defaultRecordingSourceDirectory() const
{
    return getConfig()["recordingmanager"].toObject()["defaultrecordingdir"].toString();
}

// ----------------------------------------------------------------------------
const QHash<QString, QDir>& RecordingManager::recordingSources() const
{
    return recordingSources_;
}

// ----------------------------------------------------------------------------
bool RecordingManager::addRecordingSource(const QString& name, const QDir& path)
{
    if (recordingSources_.contains(name))
        return false;

    recordingSources_.insert(name, path);
    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerRecordingSourceAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool RecordingManager::changeRecordingSourceName(const QString& oldName, const QString& newName)
{
    auto it = recordingSources_.find(oldName);
    if (it == recordingSources_.end())
        return false;
    if (recordingSources_.contains(newName))
        return false;

    QDir dir = it.value();
    recordingSources_.insert(newName, dir);
    recordingSources_.erase(it);

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerRecordingSourceNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool RecordingManager::removeRecordingSource(const QString& name)
{
    if (recordingSources_.remove(name) == 0)
        return false;

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerRecordingSourceRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
const QHash<QString, QDir>& RecordingManager::videoSources()
{
    return videoDirectories_;
}

// ----------------------------------------------------------------------------
bool RecordingManager::addVideoSource(const QString& name, const QDir& path)
{
    if (videoDirectories_.contains(name))
        return false;

    videoDirectories_.insert(name, path);
    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerVideoSourceAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool RecordingManager::changeVideoSourceName(const QString& oldName, const QString& newName)
{
    auto it = videoDirectories_.find(oldName);
    if (it == videoDirectories_.end())
        return false;
    if (videoDirectories_.contains(newName))
        return false;

    QDir dir = it.value();
    videoDirectories_.insert(newName, dir);
    videoDirectories_.erase(it);

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerVideoSourceNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool RecordingManager::removeVideoSource(const QString& name)
{
    if (videoDirectories_.remove(name) == 0)
        return false;

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerVideoSourceRemoved, name);
    return true;
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
void RecordingManager::scanForRecordings()
{
    RecordingGroup* allGroup = allRecordingGroup();
    allGroup->removeAllFiles();
    for (const auto& recdir : recordingSources_)
        for (const auto& file : recdir.entryList({"*.uhr", "*.UHR"}, QDir::Files))
            allGroup->addFile(QFileInfo(recdir, file));
}

// ----------------------------------------------------------------------------
void RecordingManager::setDefaultRecordingSourceDirectory(const QDir& path)
{
    QString canonicalPath = path.canonicalPath();

    QJsonObject& cfg = getConfig();
    QJsonObject cfgRecMgr = cfg["recordingmanager"].toObject();
    cfgRecMgr["defaultrecordingdir"] = canonicalPath;
    cfg = cfgRecMgr;
    saveConfig();

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerDefaultRecordingLocationChanged, QDir(canonicalPath));
}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName)
{
}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& name)
{

}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& name)
{

}

}
