#include "application/Util.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "application/listeners/SavedGameSessionManagerListener.hpp"

#include <QStandardPaths>
#include <QJsonObject>

namespace uhapp {

// ----------------------------------------------------------------------------
SavedGameSessionManager::SavedGameSessionManager(Config* config)
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
    savedGameSessionDirectories_.insert("Default", defaultRecordingSourceDirectory());

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    groups_.emplace("All", std::make_unique<SavedGameSessionGroup>("All"));

    scanForRecordings();
}

// ----------------------------------------------------------------------------
QDir SavedGameSessionManager::defaultRecordingSourceDirectory() const
{
    return getConfig()["SavedGameSessionManager"].toObject()["defaultrecordingdir"].toString();
}

// ----------------------------------------------------------------------------
const QHash<QString, QDir>& SavedGameSessionManager::savedGameSessionSources() const
{
    return savedGameSessionDirectories_;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::addSavedGameSessionSource(const QString& name, const QDir& path)
{
    if (savedGameSessionDirectories_.contains(name))
        return false;

    savedGameSessionDirectories_.insert(name, path);
    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerGameSessionSourceAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::changeSavedGameSessionSourceName(const QString& oldName, const QString& newName)
{
    auto it = savedGameSessionDirectories_.find(oldName);
    if (it == savedGameSessionDirectories_.end())
        return false;
    if (savedGameSessionDirectories_.contains(newName))
        return false;

    QDir dir = it.value();
    savedGameSessionDirectories_.insert(newName, dir);
    savedGameSessionDirectories_.erase(it);

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerGameSessionSourceNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::removeSavedGameSessionSource(const QString& name)
{
    if (savedGameSessionDirectories_.remove(name) == 0)
        return false;

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerGameSessionSourceRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
const QHash<QString, QDir>& SavedGameSessionManager::videoSources()
{
    return videoDirectories_;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::addVideoSource(const QString& name, const QDir& path)
{
    if (videoDirectories_.contains(name))
        return false;

    videoDirectories_.insert(name, path);
    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerVideoSourceAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::changeVideoSourceName(const QString& oldName, const QString& newName)
{
    auto it = videoDirectories_.find(oldName);
    if (it == videoDirectories_.end())
        return false;
    if (videoDirectories_.contains(newName))
        return false;

    QDir dir = it.value();
    videoDirectories_.insert(newName, dir);
    videoDirectories_.erase(it);

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerVideoSourceNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::removeVideoSource(const QString& name)
{
    if (videoDirectories_.remove(name) == 0)
        return false;

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerVideoSourceRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
SavedGameSessionGroup* SavedGameSessionManager::savedGameSessionGroup(const QString& name) const
{
    auto it = groups_.find(name.toStdString());
    if (it == groups_.end())
        return nullptr;
    return it->second.get();
}

// ----------------------------------------------------------------------------
SavedGameSessionGroup* SavedGameSessionManager::allSavedGameSessionGroup() const
{
    return groups_.find("All")->second.get();
}

// ----------------------------------------------------------------------------
const std::unordered_map<std::string, std::unique_ptr<SavedGameSessionGroup>>& SavedGameSessionManager::savedGameSessionGroups() const
{
    return groups_;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::addSavedGameSessionGroup(const QString& name)
{
    if (name.length() == 0)
        return false;

    auto it = groups_.emplace(name.toStdString(), std::make_unique<SavedGameSessionGroup>(name));
    if (it.second == false)
        return false;

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerGroupAdded, it.first->second.get());
    return true;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::renameSavedGameSessionGroup(const QString& oldName, const QString& newName)
{
    if (newName.length() == 0)
        return false;

    auto result = groups_.emplace(newName.toStdString(), std::make_unique<SavedGameSessionGroup>(nullptr));
    if (result.second == false)
        return false;

    auto newIt = result.first;
    auto oldIt = groups_.find(oldName.toStdString());
    assert(oldIt != groups_.end());

    oldIt->second->setName(newName);
    newIt->second = std::move(oldIt->second);

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerGroupNameChanged, newIt->second.get(), oldName, newName);
    groups_.erase(oldIt);
    return true;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionManager::removeSavedGameSessionGroup(const QString& name)
{
    auto it = groups_.find(name.toStdString());
    if (it == groups_.end())
        return false;

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerGroupRemoved, it->second.get());
    groups_.erase(it);
    return true;
}

// ----------------------------------------------------------------------------
void SavedGameSessionManager::scanForRecordings()
{
    SavedGameSessionGroup* allGroup = allSavedGameSessionGroup();
    allGroup->removeAllFiles();
    for (const auto& recdir : savedGameSessionDirectories_)
        for (const auto& file : recdir.entryList({"*.uhr", "*.UHR"}, QDir::Files))
            allGroup->addFile(QFileInfo(recdir, file));
}

// ----------------------------------------------------------------------------
void SavedGameSessionManager::setDefaultRecordingSourceDirectory(const QDir& path)
{
    QString canonicalPath = path.canonicalPath();

    QJsonObject& cfg = getConfig();
    QJsonObject cfgRecMgr = cfg["SavedGameSessionManager"].toObject();
    cfgRecMgr["defaultrecordingdir"] = canonicalPath;
    cfg = cfgRecMgr;
    saveConfig();

    dispatcher.dispatch(&SavedGameSessionManagerListener::onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged, QDir(canonicalPath));
}

// ----------------------------------------------------------------------------
void SavedGameSessionManager::onSavedGameSessionGroupFileAdded(SavedGameSessionGroup* group, const QFileInfo& name)
{

}

// ----------------------------------------------------------------------------
void SavedGameSessionManager::onSavedGameSessionGroupFileRemoved(SavedGameSessionGroup* group, const QFileInfo& name)
{

}

}
