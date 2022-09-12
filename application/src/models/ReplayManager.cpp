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
    json& jDefaultGamePath = jReplayManager["defaultgamepath"];
    json& jDefaultTrainingPath = jReplayManager["defaulttrainingpath"];

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
    for (const auto& jPath : jGamePaths)
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        gamePaths_.insert(QString::fromLocal8Bit(path.c_str()));
    }
    for (const auto& jPath : jTrainingPaths)
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        trainingPaths_.insert(QString::fromLocal8Bit(path.c_str()));
    }
    for (const auto& jPath : jVideoPaths)
    {
        if (jPath.is_string() == false)
            continue;
        const std::string path = jPath.get<std::string>();
        if (path.length() == 0)
            continue;
        videoPaths_.insert(QString::fromLocal8Bit(path.c_str()));
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
                it.value()->addFile(QString::fromLocal8Bit(fileName.get<std::string>().c_str()));
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

    for (const auto& it : group->fileNames())
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
    QString absFilePath = QString::fromLocal8Bit(resolveGameFile(oldFileName.toLocal8Bit().constData()).cStr());
    if (absFilePath.length() == 0)
    {
        log->error("Failed to resolve file path to \"%s\"", oldFileName.toLocal8Bit().constData());
        return false;
    }

    QFileInfo oldFile(absFilePath);
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
        if (group->removeFile(oldFileName))
            group->addFile(newFile.fileName());

    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::deleteReplay(const QString& fileName)
{
    assert(QDir(fileName).isRelative());

    bool success = true;

    rfcommon::Log* log = rfcommon::Log::root();
    QString absFilePath = QString::fromLocal8Bit(resolveGameFile(fileName.toLocal8Bit().constData()).cStr());
    if (absFilePath.length() == 0)
    {
        log->error("Failed to resolve file path to \"%s\"", fileName.toLocal8Bit().constData());
        return false;
    }

    QDir dir(QFileInfo(absFilePath).path());
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
        group->removeFile(fileName);
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
    if (session->save(fileInfo.absoluteFilePath().toLocal8Bit()))
    {
        // Add the session to the "All" recording group
        allReplayGroup()->addFile(fileInfo.fileName());
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
void ReplayManager::updateConfig()
{
    json& cfg = getConfig();
    json& jReplayManager = cfg["replaymanager"];
    
    jReplayManager["defaultgamepath"] = defaultGamePath_.absolutePath().toLocal8Bit().constData();
    jReplayManager["defaulttrainingpath"] = defaultTrainingPath_.absolutePath().toLocal8Bit().constData();

    json jGamePaths = json::array();
    for (const QDir& dir : gamePaths_)
        jGamePaths.push_back(dir.absolutePath().toLocal8Bit().constData());
    jReplayManager["gamepaths"] = jGamePaths;

    json jTrainingPaths = json::array();
    for (const QDir& dir : trainingPaths_)
        jTrainingPaths.push_back(dir.absolutePath().toLocal8Bit().constData());
    jReplayManager["trainingpaths"] = jTrainingPaths;

    json jVideoPaths = json::array();
    for (const QDir& dir : videoPaths_)
        jVideoPaths.push_back(dir.absolutePath().toLocal8Bit().constData());
    jReplayManager["videopaths"] = jVideoPaths;

    json jGroups = json::object();
    for (auto it = groups_.begin(); it != groups_.end(); ++it)
    {
        json jFiles = json::array();
        for (const auto& file : it.value()->fileNames())
            jFiles.push_back(file.toLocal8Bit().constData());
        jGroups[it.key().toUtf8().constData()] = jFiles;
    }
    jReplayManager["groups"] = jGroups;
}

// ----------------------------------------------------------------------------
rfcommon::String ReplayManager::resolveGameFile(const char* fileName) const
{
    for (auto dir : gamePaths_)
    {
        QString name = QString::fromLocal8Bit(fileName);
        if (dir.exists(name))
            return dir.absoluteFilePath(name).toLocal8Bit().constData();
    }

    return "";
}

// ----------------------------------------------------------------------------
rfcommon::String ReplayManager::resolveVideoFile(const char* fileName) const
{
    for (auto dir : videoPaths_)
    {
        QString name = QString::fromLocal8Bit(fileName);
        if (dir.exists(name))
            return dir.absoluteFilePath(name).toLocal8Bit().constData();
    }

    return "";
}

}
