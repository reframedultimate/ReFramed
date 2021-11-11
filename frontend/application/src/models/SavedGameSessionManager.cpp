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
    recordingGroups_.emplace("All", std::make_unique<RecordingGroup>("All"));

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
RecordingGroup* RecordingManager::recordingGroup(const QString& name) const
{
    auto it = recordingGroups_.find(name.toStdString());
    if (it == recordingGroups_.end())
        return nullptr;
    return it->second.get();
}

// ----------------------------------------------------------------------------
RecordingGroup* RecordingManager::allRecordingGroup() const
{
    return recordingGroups_.find("All")->second.get();
}

// ----------------------------------------------------------------------------
const std::unordered_map<std::string, std::unique_ptr<RecordingGroup>>& RecordingManager::recordingGroups() const
{
    return recordingGroups_;
}

// ----------------------------------------------------------------------------
bool RecordingManager::addRecordingGroup(const QString& name)
{
    if (name.length() == 0)
        return false;

    auto it = recordingGroups_.emplace(name.toStdString(), std::make_unique<RecordingGroup>(name));
    if (it.second == false)
        return false;

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerGroupAdded, it.first->second.get());
    return true;
}

// ----------------------------------------------------------------------------
bool RecordingManager::renameRecordingGroup(const QString& oldName, const QString& newName)
{
    if (newName.length() == 0)
        return false;

    auto result = recordingGroups_.emplace(newName.toStdString(), std::make_unique<RecordingGroup>(nullptr));
    if (result.second == false)
        return false;

    auto newIt = result.first;
    auto oldIt = recordingGroups_.find(oldName.toStdString());
    assert(oldIt != recordingGroups_.end());

    oldIt->second->setName(newName);
    newIt->second = std::move(oldIt->second);

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerGroupNameChanged, newIt->second.get(), oldName, newName);
    recordingGroups_.erase(oldIt);
    return true;
}

// ----------------------------------------------------------------------------
bool RecordingManager::removeRecordingGroup(const QString& name)
{
    auto it = recordingGroups_.find(name.toStdString());
    if (it == recordingGroups_.end())
        return false;

    dispatcher.dispatch(&RecordingManagerListener::onRecordingManagerGroupRemoved, it->second.get());
    recordingGroups_.erase(it);
    return true;
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
void RecordingManager::onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& name)
{

}

// ----------------------------------------------------------------------------
void RecordingManager::onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& name)
{

}

}
