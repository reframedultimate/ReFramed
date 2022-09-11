#include "application/Util.hpp"
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/TrainingMetaData.hpp"

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
    json& jDefaultGamePath = jGamePaths["Default"];
    json& jDefaultTrainingPath = jTrainingPaths["Default"];

    // Ensure the "Default" locations always exist
    if (jDefaultGamePath.is_string() == false || jDefaultGamePath.get<std::string>().length() == 0)
    {
        QDir path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/replays/games";
        jDefaultGamePath = QDir(path).absolutePath().toLocal8Bit().constData();
    }
    if (jDefaultTrainingPath.is_string() == false || jDefaultTrainingPath.get<std::string>().length() == 0)
    {
        QDir path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/replays/training";
        jDefaultTrainingPath = QDir(path).absolutePath().toLocal8Bit().constData();
    }

    // Populate all search paths
    defaultGamePath_ = QString::fromLocal8Bit(jDefaultGamePath.get<std::string>().c_str());
    defaultTrainingPath_ = QString::fromLocal8Bit(jDefaultTrainingPath.get<std::string>().c_str());
    for (const auto& [name, jPath] : jGamePaths.items())
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        gamePaths_.insert(QString::fromLocal8Bit(name.c_str()), QString::fromLocal8Bit(path.c_str()));
    }
    for (const auto& [name, jPath] : jTrainingPaths.items())
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        trainingPaths_.insert(QString::fromLocal8Bit(name.c_str()), QString::fromLocal8Bit(path.c_str()));
    }
    for (const auto& [name, jPath] : jVideoPaths.items())
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        videoPaths_.insert(QString::fromLocal8Bit(name.c_str()), QString::fromLocal8Bit(path.c_str()));
    }

    // The "all" recording group can't be changed or deleted and contains all
    // accessible recordings
    allGroup_.reset(new ReplayGroup("All"));
    scanForReplays();

    for (const auto& [name, fileNames] : jReplayManager["groups"].items())
    {
        auto pair = groups_.emplace(name, std::make_unique<ReplayGroup>(name.c_str()));
        if (pair.second == false)
            continue;

        if (fileNames.is_array() == false)
            continue;

        ReplayGroup* group = pair.first->second.get();
        for (const auto& fileName : fileNames)
            group;
    }
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

    if (gamePathExists(path) == false)
    {
        QString unusedName = "Default";
        for (int i = 2; i < 1e9; ++i)
        {
            if (gamePathNameExists(unusedName) == false)
                break;
            unusedName = "Default" + QString::number(i);
        }

        addGamePath(unusedName, path);
    }

    defaultGamePath_ = path;
    updateAndSaveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerDefaultGamePathChanged, path);
}

// ----------------------------------------------------------------------------
QDir ReplayManager::defaultTrainingPath() const
{
    PROFILE(ReplayManager, defaultTrainingPath);
    return defaultTrainingPath_;
}

// ----------------------------------------------------------------------------
bool ReplayManager::addGamePath(const QString& name, const QDir& path)
{
    PROFILE(ReplayManager, addGamePath);

    if (gamePaths_.contains(name))
        return false;

    gamePaths_.insert(name, path);
    updateAndSaveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGamePathAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeGamePathName(const QString& oldName, const QString& newName)
{
    PROFILE(ReplayManager, changeGamePathName);

    auto it = gamePaths_.find(oldName);
    if (it == gamePaths_.end())
        return false;
    if (gamePaths_.contains(newName))
        return false;

    QDir dir = it.value();
    gamePaths_.erase(it);
    gamePaths_.insert(newName, dir);
    updateAndSaveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGamePathNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeGamePath(const QString& name, const QDir& newPath)
{
    PROFILE(ReplayManager, changeGamePath);

    auto it = gamePaths_.find(name);
    if (it == gamePaths_.end())
        return false;

    QDir oldPath = it.value();
    *it = newPath;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGamePathChanged, name, oldPath, newPath);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeGamePath(const QString& name)
{
    PROFILE(ReplayManager, removeGamePath);

    if (gamePaths_.remove(name) == 0)
        return false;

    updateAndSaveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGamePathRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
int ReplayManager::gamePathCount() const
{
    PROFILE(ReplayManager, gamePathCount);

    return gamePaths_.size();
}

// ----------------------------------------------------------------------------
QString ReplayManager::gamePathName(int idx) const
{
    PROFILE(ReplayManager, gamePathName);

    for (auto it = gamePaths_.begin(); it != gamePaths_.end(); ++it)
        if (idx-- == 0)
            return it.key();

    std::terminate();
}

// ----------------------------------------------------------------------------
QString ReplayManager::gamePathName(const QDir& path) const
{
    PROFILE(ReplayManager, gamePathName);

    for (auto it = gamePaths_.begin(); it != gamePaths_.end(); ++it)
        if (path == it.value())
            return it.key();

    std::terminate();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::gamePath(int idx) const
{
    PROFILE(ReplayManager, gamePath);

    for (const auto& dir : gamePaths_)
        if (idx-- == 0)
            return dir;

    std::terminate();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::gamePath(const QString& name) const
{
    PROFILE(ReplayManager, gamePath);
    assert(gamePaths_.contains(name));
    return gamePaths_.find(name).value();
}

// ----------------------------------------------------------------------------
bool ReplayManager::gamePathNameExists(const QString& name) const
{
    PROFILE(ReplayManager, gamePathNameExists);
    return gamePaths_.contains(name);
}

// ----------------------------------------------------------------------------
bool ReplayManager::gamePathExists(const QDir& path) const
{
    PROFILE(ReplayManager, gamePathExists);

    for (auto it = gamePaths_.begin(); it != gamePaths_.end(); ++it)
        if (path == it.value())
            return true;
    return false;
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

    auto it = groups_.emplace(name.toStdString(), std::make_unique<ReplayGroup>(name));
    if (it.second == false)
        return nullptr;

    updateAndSaveConfig();
    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupAdded, it.first->second.get());
    return it.first->second.get();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::duplicateReplayGroup(ReplayGroup* group, const QString& newName)
{
    PROFILE(ReplayManager, duplicateReplayGroup);

    ReplayGroup* newGroup = addReplayGroup(newName);
    if (newGroup == nullptr)
        return nullptr;

    for (const auto& it : group->fileNames())
        newGroup->addFile(it);

    updateAndSaveConfig();

    return newGroup;
}

// ----------------------------------------------------------------------------
bool ReplayManager::renameReplayGroup(ReplayGroup* group, const QString& newName)
{
    PROFILE(ReplayManager, renameReplayGroup);

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
    updateAndSaveConfig();
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeReplayGroup(ReplayGroup* group)
{
    PROFILE(ReplayManager, removeReplayGroup);

    auto it = groups_.find(group->name().toStdString());
    if (it == groups_.end())
        return false;

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerGroupRemoved, it->second.get());
    groups_.erase(it);
    updateAndSaveConfig();
    return true;
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::replayGroup(const QString& name) const
{
    PROFILE(ReplayManager, replayGroup);

    auto it = groups_.find(name.toStdString());
    if (it == groups_.end())
        return nullptr;
    return it->second.get();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::replayGroup(int idx) const
{
    PROFILE(ReplayManager, replayGroup);

    for (const auto& it : groups_)
    {
        if (idx-- == 0)
            return it.second.get();
    }

    std::terminate();
}

// ----------------------------------------------------------------------------
int ReplayManager::replayGroupsCount() const
{
    PROFILE(ReplayManager, replayGroupsCount);

    return groups_.size();
}

// ----------------------------------------------------------------------------
ReplayGroup* ReplayManager::allReplayGroup() const
{
    PROFILE(ReplayManager, allReplayGroup);

    return allGroup_.get();
}

// ----------------------------------------------------------------------------
bool ReplayManager::addVideoPath(const QString& name, const QDir& path)
{
    PROFILE(ReplayManager, addVideoPath);

    if (videoPaths_.contains(name))
        return false;

    videoPaths_.insert(name, path);
    updateAndSaveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoPathAdded, name, path);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeVideoPathName(const QString& oldName, const QString& newName)
{
    PROFILE(ReplayManager, changeVideoPathName);

    auto it = videoPaths_.find(oldName);
    if (it == videoPaths_.end())
        return false;
    if (videoPaths_.contains(newName))
        return false;

    QDir dir = it.value();
    videoPaths_.insert(newName, dir);
    videoPaths_.erase(it);
    updateAndSaveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoPathNameChanged, oldName, newName);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::changeVideoPath(const QString& name, const QDir& newPath)
{
    PROFILE(ReplayManager, changeVideoPath);

    auto it = videoPaths_.find(name);
    if (it == videoPaths_.end())
        return false;

    QDir oldPath = it.value();
    *it = newPath;
    updateAndSaveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoPathChanged, name, oldPath, newPath);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::removeVideoPath(const QString& name)
{
    PROFILE(ReplayManager, removeVideoPath);

    if (videoPaths_.remove(name) == 0)
        return false;
    updateAndSaveConfig();

    dispatcher.dispatch(&ReplayManagerListener::onReplayManagerVideoPathRemoved, name);
    return true;
}

// ----------------------------------------------------------------------------
int ReplayManager::videoPathCount() const
{
    PROFILE(ReplayManager, videoPathCount);

    return videoPaths_.size();
}

// ----------------------------------------------------------------------------
QString ReplayManager::videoPathName(int idx) const
{
    PROFILE(ReplayManager, videoPathName);

    for (auto it = videoPaths_.begin(); it != videoPaths_.end(); ++it)
        if (idx-- == 0)
            return it.key();

    std::terminate();
}

// ----------------------------------------------------------------------------
QDir ReplayManager::videoPath(int idx) const
{
    PROFILE(ReplayManager, videoPath);

    for (const auto& dir : videoPaths_)
        if (idx-- == 0)
            return dir;

    std::terminate();
}

// ----------------------------------------------------------------------------
bool ReplayManager::videoPathNameExists(const QString& name) const
{
    PROFILE(ReplayManager, videoPathNameExists);
    return videoPaths_.contains(name);
}

// ----------------------------------------------------------------------------
bool ReplayManager::videoPathExists(const QDir& path) const
{
    PROFILE(ReplayManager, videoPathExists);

    for (const auto& it : videoPaths_)
        if (it == path)
            return true;

    return false;
}

// ----------------------------------------------------------------------------
QString ReplayManager::composeFileName(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata, QString formatString)
{
    PROFILE(ReplayManager, composeFileName);

    using namespace rfcommon;

    // We absolutely need the metadata, otherwise there's no way to create
    // any reasonable filename
    if (mdata == nullptr)
        return "";
    auto stampMs = mdata->timeStarted().millisSinceEpoch();
    const auto date = QDateTime::fromMSecsSinceEpoch(stampMs);

    // If there is no metadata, then we simply name the file after the date
    if (mdata == nullptr)
        return date.toString("yyyy-MM-dd HH:mm:ss.rfr");

    // Training mode case
    if (mdata->type() == MetaData::TRAINING)
    {
        const TrainingMetaData* trainingMeta = static_cast<const TrainingMetaData*>(mdata);
        QString sessionNumber = QString::number(trainingMeta->sessionNumber().value());

        // We assume there are only 2 fighters: Player and CPU
        // Try to get fighter names
        if (map)
        {
            const char* p1char = map->fighter.toName(mdata->fighterID(0));
            const char* p2char = map->fighter.toName(mdata->fighterID(1));

            return formatString
                    .replace("%date", date.toString("yyyy-MM-dd"))
                    .replace("%session", sessionNumber)
                    .replace("%p1name", mdata->name(0).cStr())
                    .replace("%p1char", p1char)
                    .replace("%p2char", p2char)
                    + ".rfr";
        }

        // Fallback filename for when there is no mapping info
        return date.toString("yyyy-MM-dd")
                + " - Training - "
                + mdata->name(0).cStr()
                + " vs CPU Session "
                + sessionNumber
                + ".rfr";
    }

    // Game session case
    if (mdata->type() == MetaData::GAME)
    {
        const GameMetaData* gameMeta = mdata->asGame();

        QString formatDesc = gameMeta->setFormat().shortDescription();
        QString setNumber = QString::number(gameMeta->setNumber().value());
        QString gameNumber = QString::number(gameMeta->gameNumber().value());

        // Create a list of character names if possible
        QStringList characters;
        for (int i = 0; i != mdata->fighterCount(); ++i)
            characters.append(map ? map->fighter.toName(mdata->fighterID(i)) : "");

        // 1v1 case
        if (mdata->fighterCount() == 2)
        {
            return formatString
                .replace("%date", date.toString("yyyy-MM-dd"))
                .replace("%format", formatDesc)
                .replace("%set", setNumber)
                .replace("%game", gameNumber)
                .replace("%p1name", gameMeta->name(0).cStr())
                .replace("%p2name", gameMeta->name(1).cStr())
                .replace("%p1char", characters[0])
                .replace("%p2char", characters[1])
                + ".rfr";
        }
        else
        {
            QStringList playerList;
            for (int i = 0; i < mdata->fighterCount(); ++i)
                playerList.append(QString(mdata->name(i).cStr()) + " (" + characters[i] + ")");
            QString players = playerList.join(" vs ");

            return date.toString("yyyy-MM-dd")
                    + " - "
                    + formatDesc
                    + " ("
                    + setNumber
                    + ") - "
                    + players
                    + " Game "
                    + gameNumber
                    + ".rfr";
        }
    }

    return "";
}

// ----------------------------------------------------------------------------
bool ReplayManager::findFreeSetAndGameNumbers(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    PROFILE(ReplayManager, findFreeSetAndGameNumbers);

    auto type = mdata ? mdata->type() : rfcommon::MetaData::GAME;
    const char* formatStr = type == rfcommon::MetaData::GAME ?
                "%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game" :
                "%date - Training - %p1name (%p1char) on %p2char Session %session";

    const QDir& dir = defaultGamePath();
    while (true)
    {
        QString fileName = composeFileName(map, mdata, formatStr);
        if (fileName == "")
            return false;
        if (QFileInfo::exists(dir.absoluteFilePath(fileName)) == false)
            return true;

        if (mdata == nullptr)
            return false;

        switch (mdata->type())
        {
            case rfcommon::MetaData::GAME: {
                auto gameMeta = static_cast<rfcommon::GameMetaData*>(mdata);
                switch (gameMeta->setFormat().type())
                {
                    case rfcommon::SetFormat::FRIENDLIES:
                    case rfcommon::SetFormat::OTHER:
                        gameMeta->setGameNumber(gameMeta->gameNumber() + 1);
                        break;

                    case rfcommon::SetFormat::BO3:
                    case rfcommon::SetFormat::BO5:
                    case rfcommon::SetFormat::BO7:
                    case rfcommon::SetFormat::FT5:
                    case rfcommon::SetFormat::FT10:
                        gameMeta->setSetNumber(gameMeta->setNumber() + 1);
                        break;
                }
            } break;

            case rfcommon::MetaData::TRAINING: {
                auto trainingMeta = static_cast<rfcommon::TrainingMetaData*>(mdata);
                trainingMeta->setSessionNumber(trainingMeta->sessionNumber() + 1);
            } break;
        }
    }
}

// ----------------------------------------------------------------------------
static int tmpCounter = 0;
bool ReplayManager::saveReplayOver(rfcommon::Session* session, const QString& oldFileName)
{
    PROFILE(ReplayManager, saveReplayOver);
    assert(QDir(oldFileName).isRelative());

    rfcommon::Log* log = rfcommon::Log::root();
    auto absFilePath = resolveGameFile(oldFileName.toLocal8Bit().constData());
    if (absFilePath.length() == 0)
    {
        log->error("Failed to resolve file path to \"%s\"", oldFileName.toLocal8Bit().constData());
        return false;
    }

    QFileInfo oldFile(absFilePath.cStr());
    QString tmpFile = "." + oldFileName + ".tmp" + QString::number(tmpCounter++);
    QDir dir(oldFile.path());

    // Determine format string based on type
    auto mdata = session->tryGetMetaData();
    auto type = mdata ? mdata->type() : rfcommon::MetaData::GAME;
    const char* formatStr = type == rfcommon::MetaData::GAME ?
                "%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game" :
                "%date - Training - %p1name (%p1char) on %p2char Session %session";

    QFileInfo newFile(dir, composeFileName(session->tryGetMappingInfo(), mdata, formatStr));

    if (allReplayGroup()->isInGroup(oldFileName) == false)
    {
        log->error("Attempted to save replay over \"%s\", but file did not exist. Aborting", oldFileName.toLocal8Bit().constData());
        return false;
    }

    log->info("Renaming %s -> %s", oldFile.absoluteFilePath().toLocal8Bit().constData(), newFile.absoluteFilePath().toLocal8Bit().constData());
    log->info("Dir: %s", dir.absolutePath().toLocal8Bit().constData());

    // Allow overwriting the same file, but disallow overwriting a different file
    // that already exists
    if (oldFile != newFile)
    {
        if (newFile.exists())
        {
            log->warning("File %s already exists, aborting rename", newFile.absoluteFilePath().toLocal8Bit().constData());
            return false;
        }
    }

    if (dir.rename(oldFileName, tmpFile) == false)
    {
        log->error("Failed to rename %s -> %s", oldFileName.toLocal8Bit().constData(), tmpFile.toLocal8Bit().constData());
        return false;
    }

    QByteArray ba = newFile.absoluteFilePath().toLocal8Bit();
    if (session->save(ba.constData()) == false)
    {
        log->error("Failed to save session to %s", ba.constData());
        if (dir.rename(tmpFile, oldFileName))
            log->error("Failed to rename %s -> %s", tmpFile.toLocal8Bit().constData(), oldFileName.toLocal8Bit().constData());

        return false;
    }

    if (dir.remove(tmpFile) == false)
    {
        // On Windows, it's not possible to delete a file when it is memory mapped,
        // which is the case with session files. What we can do is set the FILE_FLAG_DELETE_ON_CLOSE
        // flag on the file, which will cause it to be deleted as soon as all references
        // to the session are dropped
        QByteArray ba = dir.absoluteFilePath(tmpFile).toLocal8Bit();
        if (rfcommon::MappedFile::setDeleteOnClose(ba.constData()) == false)
            log->error("Failed to remove file %s");
    }

    if (allGroup_->removeFile(oldFileName))
        allGroup_->addFile(newFile.fileName());
    for (auto& group : groups_)
        if (group.second->removeFile(oldFileName))
            group.second->addFile(newFile.fileName());

    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::deleteReplay(const QString& fileName)
{
    assert(QDir(fileName).isRelative());

    bool success = true;

    rfcommon::Log* log = rfcommon::Log::root();
    auto absFilePath = resolveGameFile(fileName.toLocal8Bit().constData());
    if (absFilePath.length() == 0)
    {
        log->error("Failed to resolve file path to \"%s\"", fileName.toLocal8Bit().constData());
        return false;
    }

    QDir dir(QFileInfo(absFilePath.cStr()).path());
    QString tmpFile = "." + fileName + ".tmp" + QString::number(tmpCounter++);

    if (dir.rename(fileName, tmpFile) == false)
    {
        log->error("Failed to rename %s -> %s", fileName.toLocal8Bit().constData(), tmpFile.toLocal8Bit().constData());
        return false;
    }

    if (dir.remove(tmpFile) == false)
    {
        // On Windows, it's not possible to delete a file when it is memory mapped,
        // which is the case with session files. What we can do is set the FILE_FLAG_DELETE_ON_CLOSE
        // flag on the file, which will cause it to be deleted as soon as all references
        // to the session are dropped
        QByteArray ba = dir.absoluteFilePath(tmpFile).toLocal8Bit();
        if (rfcommon::MappedFile::setDeleteOnClose(ba.constData()) == false)
        {
            log->error("Failed to remove file %s", ba.constData());
            success = false;
        }
    }

    // Delete from group, regardless of whether the file was successfully deleted or not
    for (auto& group : groups_)
        group.second->removeFile(fileName);
    allGroup_->removeFile(fileName);

    return success;
}

// ----------------------------------------------------------------------------
bool ReplayManager::saveReplayWithDefaultSettings(rfcommon::Session* session)
{
    PROFILE(ReplayManager, saveReplayWithDefaultSettings);

    auto map = session->tryGetMappingInfo();
    auto mdata = session->tryGetMetaData();
    auto type = mdata ? mdata->type() : rfcommon::MetaData::GAME;

    // Determine target directory based on type
    QDir dir = type == rfcommon::MetaData::TRAINING ?
            defaultTrainingPath() : defaultGamePath();

    // Determine format string based on type
    const char* formatStr = type == rfcommon::MetaData::GAME ?
                "%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game" :
                "%date - Training - %p1name (%p1char) on %p2char Session %session";

    if (dir.exists() == false)
        dir.mkpath(".");
    QFileInfo fileInfo(dir, composeFileName(map, mdata, formatStr));
    QByteArray ba = fileInfo.absoluteFilePath().toLocal8Bit();
    if (session->save(ba.constData()))
    {
        // Add the session to the "All" recording group
        allReplayGroup()->addFile(fileInfo.absoluteFilePath());
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
        for (const auto& file : replayDir.entryList({"*.rfr"}, QDir::Files))
            allGroup->addFile(file);
    }
}
// ----------------------------------------------------------------------------
void ReplayManager::updateAndSaveConfig()
{
    json& cfg = getConfig();
    json& jReplayManager = cfg["replaymanager"];
    
    jReplayManager["defaultgamepath"] = defaultGamePath_.absolutePath().toLocal8Bit().constData();
    jReplayManager["defaulttrainingpath"] = defaultTrainingPath_.absolutePath().toLocal8Bit().constData();

    json jGamePaths = json::object();
    for (auto it = gamePaths_.begin(); it != gamePaths_.end(); ++it)
        jGamePaths[it.key().toLocal8Bit().constData()] = it.value().absolutePath().toLocal8Bit().constData();
    jReplayManager["gamepaths"] = jGamePaths;

    json jTrainingPaths = json::object();
    for (auto it = trainingPaths_.begin(); it != trainingPaths_.end(); ++it)
        jTrainingPaths[it.key().toLocal8Bit().constData()] = it.value().absolutePath().toLocal8Bit().constData();
    jReplayManager["trainingpaths"] = jTrainingPaths;

    json jVideoPaths = json::object();
    for (auto it = videoPaths_.begin(); it != videoPaths_.end(); ++it)
        jVideoPaths[it.key().toLocal8Bit().constData()] = it.value().absolutePath().toLocal8Bit().constData();
    jReplayManager["videopaths"] = jVideoPaths;

    json jGroups = json::object();
    for (const auto& [name, group] : groups_)
    {
        json jFiles = json::array();
        for (const auto& file : group->fileNames())
            jFiles.push_back(file.toLocal8Bit().constData());
        jGroups[name] = jFiles;
    }
    jReplayManager["groups"] = jGroups;

    saveConfig();
}

// ----------------------------------------------------------------------------
rfcommon::String ReplayManager::resolveGameFile(const char* fileName) const
{
    for (auto dir : gamePaths_)
    {
        QString absName = dir.absoluteFilePath(fileName);
        if (QFileInfo(absName).isFile())
            return absName.toLocal8Bit().constData();
    }

    return "";
}

// ----------------------------------------------------------------------------
rfcommon::String ReplayManager::resolveVideoFile(const char* fileName) const
{
    for (auto dir : videoPaths_)
    {
        QString absName = dir.absoluteFilePath(fileName);
        if (QFileInfo(absName).isFile())
            return absName.toLocal8Bit().constData();
    }

    return "";
}

}
