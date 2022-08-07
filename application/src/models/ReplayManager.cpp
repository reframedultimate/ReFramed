#include "application/Util.hpp"
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/ReplayManager.hpp"

#include <QStandardPaths>
#include <QJsonObject>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayManager::ReplayManager(Config* config)
    : ConfigAccessor(config)
{
    QJsonObject& cfg = getConfig();
    if (cfg["replaymanager"].isNull())
        cfg["replaymanager"] = QJsonObject();
    auto cfgReplayManager = cfg["replaymanager"].toObject();
    if (cfgReplayManager["defaultreplaypath"].isNull())
    {
        QDir defaultReplayPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/replays";
        cfgReplayManager["defaultreplaypath"] = QDir(defaultReplayPath).absolutePath();
    }
    cfg["replaymanager"] = cfgReplayManager;
    saveConfig();

    // Default location is always at index 0
    replayDirectories_.insert("Default", defaultReplaySourceDirectory());

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    groups_.emplace("All", std::make_unique<ReplayGroup>("All"));

    scanForReplays();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultReplaySourceDirectory() const
{
    return getConfig()["replaymanager"].toObject()["defaultreplaypath"].toString();
}

// ----------------------------------------------------------------------------
void ReplayManager::setDefaultReplaySourceDirectory(const QDir& path)
{
    QJsonObject& cfg = getConfig();
    QJsonObject cfgReplayManager = cfg["replaymanager"].toObject();
    cfgReplayManager["defaultreplaypath"] = path.absolutePath();
    cfg["replaymanager"] = cfgReplayManager;
    saveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerDefaultReplaySaveLocationChanged, path);
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultGameSessionSourceDirectory() const
{
    QDir dir = defaultReplaySourceDirectory();
    return dir.absolutePath() + "/games";
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultTrainingSessionSourceDirectory() const
{
    QDir dir = defaultReplaySourceDirectory();
    return dir.absolutePath() + "/training";
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
    replayDirectories_.erase(it);
    replayDirectories_.insert(newName, dir);

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
void ReplayManager::scanForReplays()
{
    ReplayGroup* allGroup = allReplayGroup();
    allGroup->removeAllFiles();
    for (const auto& replayDir : replayDirectories_)
    {
        QDir matchDir = replayDir.path() + "/games";
        QDir trainingDir = replayDir.path() + "/training";
        for (const auto& file : matchDir.entryList({"*.rfr"}, QDir::Files))
            allGroup->addFile(QFileInfo(matchDir, file));
        for (const auto& file : trainingDir.entryList({"*.rfr"}, QDir::Files))
            allGroup->addFile(QFileInfo(matchDir, file));
    }
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
