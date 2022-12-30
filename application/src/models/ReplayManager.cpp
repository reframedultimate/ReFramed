#include "application/Util.hpp"
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/TrainingMetadata.hpp"

#include <QDateTime>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonArray>

namespace rfapp {

using namespace nlohmann;

// ----------------------------------------------------------------------------
ReplayManager::ReplayManager(Config* config)
    : ConfigAccessor(config)
{
    json& cfg = getConfig();
    json& jReplayManager = cfg["replaymanager"];
    json& jGamePaths = jReplayManager["gamepaths"];
    json& jTrainingPaths = jReplayManager["trainingpaths"];
    json& jVideoPaths = jReplayManager["videopaths"];
    json& jDefaultGamePath = jReplayManager["defaultgamepath"];
    json& jDefaultTrainingPath = jReplayManager["defaulttrainingpath"];

    // Ensure the "Default" locations always exist
    if (jDefaultGamePath.is_string() == false || jDefaultGamePath.get<std::string>().length() == 0)
    {
        QDir path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/replays/games";
        jDefaultGamePath = QDir(path).absolutePath().toUtf8().constData();
    }
    if (jDefaultTrainingPath.is_string() == false || jDefaultTrainingPath.get<std::string>().length() == 0)
    {
        QDir path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/replays/training";
        jDefaultTrainingPath = QDir(path).absolutePath().toUtf8().constData();
    }

    // Populate all search paths
    defaultGamePath_.setPath(QString::fromUtf8(jDefaultGamePath.get<std::string>().c_str()));
    defaultTrainingPath_.setPath(QString::fromUtf8(jDefaultTrainingPath.get<std::string>().c_str()));
    for (const auto& jPath : jGamePaths)
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        gamePaths_.insert(QString::fromUtf8(path.c_str()));
    }
    for (const auto& jPath : jTrainingPaths)
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        trainingPaths_.insert(QString::fromUtf8(path.c_str()));
    }
    for (const auto& jPath : jVideoPaths)
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        videoPaths_.insert(QString::fromUtf8(path.c_str()));
    }

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    allGroup_.reset(new ReplayGroup("All"));
    scanForReplays();

    for (const auto& [name, fileNames] : jReplayManager["groups"].items())
    {
        QString qName = QString::fromUtf8(name.c_str());
        auto it = groups_.insert(qName, new ReplayGroup(qName));

        if (fileNames.is_array() == false)
            continue;

        for (const auto& fileName : fileNames)
            if (fileName.is_string())
            {
                std::string fileNameUtf8 = fileName.get<std::string>();
                it.value()->addFile(QString::fromUtf8(fileNameUtf8.c_str()));
            }
    }
}

// ----------------------------------------------------------------------------
ReplayManager::~ReplayManager()
{
    updateConfig();  // Makes sure to include changes to groups into the config

    for (ReplayGroup* group : groups_)
        delete group;
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultGamePath() const
{
    PROFILE(ReplayManager, defaultGamePath);
    return defaultGamePath_;
}

// ----------------------------------------------------------------------------
void ReplayManager::setDefaultGamePath(const QDir& path)
{
    PROFILE(ReplayManager, setDefaultGamePath);

    // Will fail if it already exists but that's fine
    addGamePath(path);

    defaultGamePath_ = path;
    updateConfig();
    saveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerDefaultGamePathChanged, path);
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultTrainingPath() const
{
    PROFILE(ReplayManager, defaultTrainingPath);
    return defaultTrainingPath_;
}

// ----------------------------------------------------------------------------
bool ReplayManager::addGamePath(const QDir& path)
{
    PROFILE(ReplayManager, addGamePath);

    if (gamePaths_.contains(path))
        return false;

    gamePaths_.insert(path);

    for (const auto& file : path.entryList({ "*.rfr" }, QDir::Files))
        allReplayGroup()->addFile(file);

    updateConfig();
    saveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGamePathAdded, path);

    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeGamePath(const QDir& path)
{
    PROFILE(ReplayManager, removeGamePath);

    if (gamePaths_.remove(path) == false)
        return false;

    for (const auto& file : path.entryList({ "*.rfr" }, QDir::Files))
        allReplayGroup()->removeFile(file);

    updateConfig();
    saveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGamePathRemoved, path);

    return true;
}

// ----------------------------------------------------------------------------
int ReplayManager::gamePathCount() const
{
    PROFILE(ReplayManager, gamePathCount);

    return gamePaths_.size();
}

// ----------------------------------------------------------------------------
const QDir& ReplayManager::gamePath(int idx) const
{
    PROFILE(ReplayManager, gamePath);

    for (const auto& dir : gamePaths_)
        if (idx-- == 0)
            return dir;

    std::terminate();
}

// ----------------------------------------------------------------------------
bool ReplayManager::gamePathExists(const QDir& path) const
{
    PROFILE(ReplayManager, gamePathExists);
    return gamePaths_.contains(path);
}

// ----------------------------------------------------------------------------
bool ReplayManager::addVideoPath(const QDir& path)
{
    PROFILE(ReplayManager, addVideoPath);

    if (videoPaths_.contains(path))
        return false;

    videoPaths_.insert(path);
    updateConfig();
    saveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoPathAdded, path);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeVideoPath(const QDir& path)
{
    PROFILE(ReplayManager, removeVideoPath);

    if (videoPaths_.remove(path) == false)
        return false;

    updateConfig();
    saveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoPathRemoved, path);
    return true;
}

// ----------------------------------------------------------------------------
int ReplayManager::videoPathCount() const
{
    PROFILE(ReplayManager, videoPathCount);

    return videoPaths_.size();
}

// ----------------------------------------------------------------------------
const QDir& ReplayManager::videoPath(int idx) const
{
    PROFILE(ReplayManager, videoPath);

    for (const auto& dir : videoPaths_)
        if (idx-- == 0)
            return dir;

    std::terminate();
}

// ----------------------------------------------------------------------------
bool ReplayManager::videoPathExists(const QDir& path) const
{
    PROFILE(ReplayManager, videoPathExists);
    return videoPaths_.contains(path);
}

// ----------------------------------------------------------------------------
QString ReplayManager::findFreeGroupName(const QString& name) const
{
    PROFILE(ReplayManager, findFreeGroupName);

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
    PROFILE(ReplayManager, addReplayGroup);

    if (name.length() == 0)
        return nullptr;
    if (groups_.contains(name))
        return nullptr;

    auto it = groups_.insert(name, new ReplayGroup(name));

    updateConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupAdded, it.value());
    return it.value();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::duplicateReplayGroup(ReplayGroup* group, const QString& newName)
{
    PROFILE(ReplayManager, duplicateReplayGroup);

    ReplayGroup* newGroup = addReplayGroup(newName);
    if (newGroup == nullptr)
        return nullptr;

    for (const auto& it : group->files())
        newGroup->addFile(it);

    updateConfig();

    return newGroup;
}

// ----------------------------------------------------------------------------
bool ReplayManager::renameReplayGroup(ReplayGroup* group, const QString& newName)
{
    PROFILE(ReplayManager, renameReplayGroup);

    if (newName.length() == 0)
        return false;

    if (groups_.contains(newName))
        return false;

    QString oldName = group->name();

    auto newIt = groups_.insert(newName, nullptr);
    auto oldIt = groups_.find(oldName);
    assert(oldIt != groups_.end());

    oldIt.value()->setName(newName);
    newIt.value() = std::move(oldIt.value());
    groups_.erase(oldIt);

    updateConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupNameChanged, group, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeReplayGroup(ReplayGroup* group)
{
    PROFILE(ReplayManager, removeReplayGroup);

    auto it = groups_.find(group->name());
    assert(it != groups_.end());
    groups_.erase(it);

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupRemoved, group);
    delete group;
    updateConfig();
    return true;
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::replayGroup(const QString& name) const
{
    PROFILE(ReplayManager, replayGroup);

    auto it = groups_.find(name);
    if (it == groups_.end())
        return nullptr;
    return it.value();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::replayGroup(int idx) const
{
    PROFILE(ReplayManager, replayGroup);

    for (ReplayGroup* group : groups_)
        if (idx-- == 0)
            return group;

    std::terminate();
}

// ----------------------------------------------------------------------------
int ReplayManager::replayGroupCount() const
{
    PROFILE(ReplayManager, replayGroupCount);

    return groups_.size();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::allReplayGroup() const
{
    PROFILE(ReplayManager, allReplayGroup);

    return allGroup_.get();
}

// ----------------------------------------------------------------------------
static int tmpCounter = 0;
bool ReplayManager::saveReplayOver(rfcommon::Session* session, const QString& oldFileName)
{
    PROFILE(ReplayManager, saveReplayOver);

    rfcommon::Log* log = rfcommon::Log::root();

    // Determine old file name
    rfcommon::String oldFilePathUtf8 = resolveGameFile(oldFileName.toUtf8().constData());
    if (oldFilePathUtf8.length() == 0)
    {
        log->error("Failed to resolve file path to \"%s\"", oldFileName.toUtf8().constData());
        return false;
    }
    QString oldFilePath = QString::fromUtf8(oldFilePathUtf8.cStr());
    QDir dir(QFileInfo(oldFilePath).path());

    // Determine temp file name
    QString tmpFileName = "." + oldFileName + ".tmp" + QString::number(tmpCounter++);

    // Determine new file name
    auto newFileNameParts = rfcommon::ReplayFileParts::fromMetadata(session->tryGetMappingInfo(), session->tryGetMetadata());
    QString newFileName = QString::fromUtf8(newFileNameParts.toFileName().cStr());

    if (allReplayGroup()->isInGroup(oldFileName) == false)
    {
        log->error("Attempted to save replay over \"%s\", but file did not exist. Aborting", oldFileName.toUtf8().constData());
        return false;
    }

    log->info("Renaming %s -> %s", dir.absoluteFilePath(oldFileName).toUtf8().constData(), dir.absoluteFilePath(newFileName).toUtf8().constData());
    log->info("Dir: %s", dir.absolutePath().toUtf8().constData());

    // Allow overwriting the same file, but disallow overwriting a different file
    // that already exists
    if (oldFileName != newFileName)
    {
        if (dir.exists(newFileName))
        {
            log->warning("File %s already exists, aborting rename", dir.absoluteFilePath(newFileName).toUtf8().constData());
            return false;
        }
    }

    if (dir.rename(oldFileName, tmpFileName) == false)
    {
        log->error("Failed to rename %s -> %s", oldFileName.toUtf8().constData(), tmpFileName.toUtf8().constData());
        return false;
    }

    QByteArray newFileNameUtf8 = dir.absoluteFilePath(newFileName).toUtf8();
    if (session->save(newFileNameUtf8.constData()) == false)
    {
        log->error("Failed to save session to %s", newFileNameUtf8.constData());
        if (dir.rename(tmpFileName, oldFileName))
            log->error("Failed to rename %s -> %s", tmpFileName.toUtf8().constData(), oldFileName.toUtf8().constData());

        return false;
    }

    if (dir.remove(tmpFileName) == false)
    {
        // On Windows, it's not possible to delete a file when it is memory mapped,
        // which is the case with session files. What we can do is set the FILE_FLAG_DELETE_ON_CLOSE
        // flag on the file, which will cause it to be deleted as soon as all references
        // to the session are dropped
        QByteArray ba = dir.absoluteFilePath(tmpFileName).toUtf8();
        if (rfcommon::MappedFile::setDeleteOnClose(ba.constData()) == false)
            log->error("Failed to remove file %s", ba.constData());
    }

    if (allGroup_->removeFile(oldFileName))
        allGroup_->addFile(newFileName);
    for (auto& group : groups_)
        if (group->removeFile(oldFileName))
            group->addFile(newFileName);

    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::deleteReplay(const QString& fileName)
{
    PROFILE(ReplayManager, deleteReplay);

    rfcommon::Log* log = rfcommon::Log::root();
    bool success = true;

    rfcommon::String filePathUtf8 = resolveGameFile(fileName.toUtf8().constData());
    if (filePathUtf8.length() == 0)
    {
        log->error("Failed to resolve file path to \"%s\"", fileName.toUtf8().constData());
        return false;
    }
    QString filePath = QString::fromUtf8(filePathUtf8.cStr());
    QDir dir(QFileInfo(filePath).path());

    QString tmpFile = "." + fileName + ".tmp" + QString::number(tmpCounter++);

    if (dir.rename(fileName, tmpFile) == false)
    {
        log->error("Failed to rename %s -> %s", fileName.toUtf8().constData(), tmpFile.toUtf8().constData());
        return false;
    }

    if (dir.remove(tmpFile) == false)
    {
        // On Windows, it's not possible to delete a file when it is memory mapped,
        // which is the case with session files. What we can do is set the FILE_FLAG_DELETE_ON_CLOSE
        // flag on the file, which will cause it to be deleted as soon as all references
        // to the session are dropped
        QByteArray ba = dir.absoluteFilePath(tmpFile).toUtf8();
        if (rfcommon::MappedFile::setDeleteOnClose(ba.constData()) == false)
        {
            log->error("Failed to remove file %s", ba.constData());
            success = false;
        }
    }

    // Delete from group, regardless of whether the file was successfully deleted or not
    for (auto& group : groups_)
        group->removeFile(fileName);
    allGroup_->removeFile(fileName);

    return success;
}

// ----------------------------------------------------------------------------
bool ReplayManager::saveReplayWithDefaultSettings(rfcommon::Session* session)
{
    PROFILE(ReplayManager, saveReplayWithDefaultSettings);

    auto map = session->tryGetMappingInfo();
    auto mdata = session->tryGetMetadata();
    auto type = mdata ? mdata->type() : rfcommon::Metadata::GAME;

    // Determine target directory based on type
    QDir dir = type == rfcommon::Metadata::TRAINING ?
            defaultTrainingPath() : defaultGamePath();

    if (dir.exists() == false)
        dir.mkpath(".");

    auto fileNameParts = rfcommon::ReplayFileParts::fromMetadata(map, mdata);
    auto fileNameUtf8 = fileNameParts.toFileName();
    auto fileName = QString::fromUtf8(fileNameUtf8.cStr());
    auto filePath = dir.absoluteFilePath(fileName);
    if (session->save(filePath.toUtf8().constData()))
    {
        // Add the session to the "All" recording group
        allReplayGroup()->addFile(fileName);
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
void ReplayManager::scanForReplays()
{
    PROFILE(ReplayManager, scanForReplays);

    ReplayGroup* allGroup = allReplayGroup();
    allGroup->removeAllFiles();
    for (const auto& replayDir : gamePaths_)
    {
        for (const auto& file : replayDir.entryList({ "*.rfr" }, QDir::Files))
            allGroup->addFile(file);
    }
}

// ----------------------------------------------------------------------------
void ReplayManager::updateConfig()
{
    PROFILE(ReplayManager, updateConfig);

    json& cfg = getConfig();
    json& jReplayManager = cfg["replaymanager"];

    jReplayManager["defaultgamepath"] = defaultGamePath_.absolutePath().toUtf8().constData();
    jReplayManager["defaulttrainingpath"] = defaultTrainingPath_.absolutePath().toUtf8().constData();

    json jGamePaths = json::array();
    for (const QDir& dir : gamePaths_)
        jGamePaths.push_back(dir.absolutePath().toUtf8().constData());
    jReplayManager["gamepaths"] = jGamePaths;

    json jTrainingPaths = json::array();
    for (const QDir& dir : trainingPaths_)
        jTrainingPaths.push_back(dir.absolutePath().toUtf8().constData());
    jReplayManager["trainingpaths"] = jTrainingPaths;

    json jVideoPaths = json::array();
    for (const QDir& dir : videoPaths_)
        jVideoPaths.push_back(dir.absolutePath().toUtf8().constData());
    jReplayManager["videopaths"] = jVideoPaths;

    json jGroups = json::object();
    for (auto it = groups_.begin(); it != groups_.end(); ++it)
    {
        json jFiles = json::array();
        for (const auto& file : it.value()->files())
            jFiles.push_back(file.toUtf8().constData());
        jGroups[it.key().toUtf8().constData()] = jFiles;
    }
    jReplayManager["groups"] = jGroups;
}

// ----------------------------------------------------------------------------
rfcommon::String ReplayManager::resolveGameFile(const char* fileName) const
{
    PROFILE(ReplayManager, resolveGameFile);

    for (auto dir : gamePaths_)
    {
        QString name = QString::fromUtf8(fileName);
        if (dir.exists(name))
            return dir.absoluteFilePath(name).toUtf8().constData();
    }

    return "";
}

// ----------------------------------------------------------------------------
rfcommon::String ReplayManager::resolveVideoFile(const char* fileName) const
{
    PROFILE(ReplayManager, resolveVideoFile);

    for (auto dir : videoPaths_)
    {
        QString name = QString::fromUtf8(fileName);
        if (dir.exists(name))
            return dir.absoluteFilePath(name).toUtf8().constData();
    }

    return "";
}

}
