#include "application/Util.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "application/listeners/SavedGameSessionManagerListener.hpp"

#include <QStandardPaths>
#include <QJsonObject>

namespace uhapp {

// ----------------------------------------------------------------------------
ReplayManager::ReplayManager(Config* config)
    : ConfigAccessor(config)
{
    QJsonObject& cfg = getConfig();
    if (cfg["SavedGameSessionManager"].isNull())
        cfg["SavedGameSessionManager"] = QJsonObject();
    auto cfgRecMgr = cfg["SavedGameSessionManager"].toObject();
    if (cfgRecMgr["defaultrecordingdir"].isNull())
        cfgRecMgr["defaultrecordingdir"] = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).canonicalPath();
    cfg["SavedGameSessionManager"] = cfgRecMgr;

    // Default location is always at index 0
    replayDirectories_.insert("Default", defaultRecordingSourceDirectory());

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    groups_.emplace("All", std::make_unique<ReplayGroup>("All"));

    scanForRecordings();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultRecordingSourceDirectory() const
{
    return getConfig()["SavedGameSessionManager"].toObject()["defaultrecordingdir"].toString();
}

// ----------------------------------------------------------------------------
void ReplayManager::setDefaultRecordingSourceDirectory(const QDir& path)
{
    QString canonicalPath = path.canonicalPath();

    QJsonObject& cfg = getConfig();
    QJsonObject cfgRecMgr = cfg["SavedGameSessionManager"].toObject();
    cfgRecMgr["defaultrecordingdir"] = canonicalPath;
    cfg = cfgRecMgr;
    saveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerDefaultReplaySaveLocationChanged, QDir(canonicalPath));
}

// ----------------------------------------------------------------------------
bool ReplayManager::addReplaySource(const QString& name, const QDir& path)
{
    if (replayDirectories_.contains(name))
        return false;

    replayDirectories_.insert(name, path);
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerReplaySourceAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeReplaySourceName(const QString& oldName, const QString& newName)
{
    auto it = replayDirectories_.find(oldName);
    if (it == replayDirectories_.end())
        return false;
    if (replayDirectories_.contains(newName))
        return false;

    QDir dir = it.value();
    replayDirectories_.insert(newName, dir);
    replayDirectories_.erase(it);

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerReplaySourceNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeReplaySourcePath(const QString& name, const QDir& newPath)
{
    auto it = replayDirectories_.find(name);
    if (it == replayDirectories_.end())
        return false;

    QDir oldPath = it.value();
    *it = newPath;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerReplaySourcePathChanged, name, oldPath, newPath);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeReplaySource(const QString& name)
{
    if (replayDirectories_.remove(name) == 0)
        return false;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerReplaySourceRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
int ReplayManager::replaySourcesCount() const
{
    return replayDirectories_.size();
}

// ----------------------------------------------------------------------------
QString ReplayManager::replaySourceName(int idx) const
{
    for (auto it = replayDirectories_.begin(); it != replayDirectories_.end(); ++it)
        if (idx-- == 0)
            return it.key();

    std::terminate();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::replaySourcePath(int idx) const
{
    for (const auto& dir : replayDirectories_)
        if (idx-- == 0)
            return dir;

    std::terminate();
}

// ----------------------------------------------------------------------------
QString ReplayManager::findFreeGroupName(const QString& name)
{
    int idx = 1;
    QString candidate = name;
    while (true)
    {
        if (replayGroup(candidate) == nullptr)
            break;

        candidate = name + " " + QString::number(idx);
        idx++;
    }

    return candidate;
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::addReplayGroup(const QString& name)
{
    if (name.length() == 0)
        return nullptr;

    auto it = groups_.emplace(name.toStdString(), std::make_unique<ReplayGroup>(name));
    if (it.second == false)
        return nullptr;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupAdded, it.first->second.get());
    return it.first->second.get();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::duplicateReplayGroup(ReplayGroup* group, const QString& newName)
{
    ReplayGroup* newGroup = addReplayGroup(newName);
    if (newGroup == nullptr)
        return nullptr;

    for (const auto& it : group->absFilePathList())
        newGroup->addFile(it);

    return newGroup;
}

// ----------------------------------------------------------------------------
bool ReplayManager::renameReplayGroup(ReplayGroup* group, const QString& newName)
{
    const QString& oldName = group->name();
    if (newName.length() == 0)
        return false;

    auto result = groups_.emplace(newName.toStdString(), std::make_unique<ReplayGroup>(nullptr));
    if (result.second == false)
        return false;

    auto newIt = result.first;
    auto oldIt = groups_.find(oldName.toStdString());
    assert(oldIt != groups_.end());

    oldIt->second->setName(newName);
    newIt->second = std::move(oldIt->second);

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupNameChanged, newIt->second.get(), oldName, newName);
    groups_.erase(oldIt);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeReplayGroup(ReplayGroup* group)
{
    auto it = groups_.find(group->name().toStdString());
    if (it == groups_.end())
        return false;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupRemoved, it->second.get());
    groups_.erase(it);
    return true;
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::replayGroup(const QString& name) const
{
    auto it = groups_.find(name.toStdString());
    if (it == groups_.end())
        return nullptr;
    return it->second.get();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::replayGroup(int idx) const
{
    for (const auto& it : groups_)
        if (idx-- == 0)
            return it.second.get();

    std::terminate();
}

// ----------------------------------------------------------------------------
int ReplayManager::replayGroupsCount() const
{
    return groups_.size();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::allReplayGroup() const
{
    return groups_.find("All")->second.get();
}

// ----------------------------------------------------------------------------
bool ReplayManager::addVideoSource(const QString& name, const QDir& path)
{
    if (videoDirectories_.contains(name))
        return false;

    videoDirectories_.insert(name, path);
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoSourceAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeVideoSourceName(const QString& oldName, const QString& newName)
{
    auto it = videoDirectories_.find(oldName);
    if (it == videoDirectories_.end())
        return false;
    if (videoDirectories_.contains(newName))
        return false;

    QDir dir = it.value();
    videoDirectories_.insert(newName, dir);
    videoDirectories_.erase(it);

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoSourceNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeVideoSourcePath(const QString& name, const QDir& newPath)
{
    auto it = videoDirectories_.find(name);
    if (it == videoDirectories_.end())
        return false;

    QDir oldPath = it.value();
    *it = newPath;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoSourcePathChanged, name, oldPath, newPath);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeVideoSource(const QString& name)
{
    if (videoDirectories_.remove(name) == 0)
        return false;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoSourceRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
int ReplayManager::videoSourcesCount() const
{
    return videoDirectories_.size();
}

// ----------------------------------------------------------------------------
QString ReplayManager::videoSourceName(int idx) const
{
    for (auto it = videoDirectories_.begin(); it != videoDirectories_.end(); ++it)
        if (idx-- == 0)
            return it.key();

    std::terminate();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::videoSourcePath(int idx) const
{
    for (const auto& dir : videoDirectories_)
        if (idx-- == 0)
            return dir;

    std::terminate();
}

// ----------------------------------------------------------------------------
void ReplayManager::scanForRecordings()
{
    ReplayGroup* allGroup = allReplayGroup();
    allGroup->removeAllFiles();
    for (const auto& recdir : replayDirectories_)
        for (const auto& file : recdir.entryList({"*.uhr", "*.UHR"}, QDir::Files))
            allGroup->addFile(QFileInfo(recdir, file));
}

// ----------------------------------------------------------------------------
void ReplayManager::onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& name)
{

}

// ----------------------------------------------------------------------------
void ReplayManager::onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& name)
{

}

}
