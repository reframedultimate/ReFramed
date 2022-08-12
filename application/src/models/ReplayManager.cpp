#include "application/Util.hpp"
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TimeStamp.hpp"

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
QString ReplayManager::composeFileName(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata, QString formatString)
{
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
        const GameMetaData* gameMeta = static_cast<const GameMetaData*>(mdata);

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
    auto type = mdata ? mdata->type() : rfcommon::MetaData::GAME;
    const char* formatStr = type == rfcommon::MetaData::GAME ?
                "%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game" :
                "%date - Training - %p1name (%p1char) on %p2char Session %session";

    const QDir& dir = defaultGameSessionSourceDirectory();
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
                    case rfcommon::SetFormat::PRACTICE:
                    case rfcommon::SetFormat::TRAINING:
                    case rfcommon::SetFormat::COACHING:
                    case rfcommon::SetFormat::OTHER:
                        gameMeta->setGameNumber(gameMeta->gameNumber() + 1);
                        break;

                    case rfcommon::SetFormat::BO3:
                    case rfcommon::SetFormat::BO3MM:
                    case rfcommon::SetFormat::BO5:
                    case rfcommon::SetFormat::BO5MM:
                    case rfcommon::SetFormat::BO7:
                    case rfcommon::SetFormat::BO7MM:
                    case rfcommon::SetFormat::FT5:
                    case rfcommon::SetFormat::FT5MM:
                    case rfcommon::SetFormat::FT10:
                    case rfcommon::SetFormat::FT10MM:
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
bool ReplayManager::saveReplayOver(rfcommon::Session* session, const QFileInfo& oldFile)
{
    QString oldFileName = oldFile.fileName();
    QDir dir(oldFile.path());

    // Determine format string based on type
    auto mdata = session->tryGetMetaData();
    auto type = mdata ? mdata->type() : rfcommon::MetaData::GAME;
    const char* formatStr = type == rfcommon::MetaData::GAME ?
                "%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game" :
                "%date - Training - %p1name (%p1char) on %p2char Session %session";

    QFileInfo newFile(dir, composeFileName(session->tryGetMappingInfo(), mdata, formatStr));

    if (dir.rename(oldFileName, "." + oldFileName) == false)
        return false;

    QByteArray ba = newFile.absoluteFilePath().toUtf8();
    if (session->save(ba.constData()) == false)
    {
        dir.rename("." + oldFileName, oldFileName);
        return false;
    }

    dir.remove("." + oldFileName);

    for (auto& group : groups_)
        if (group.second->removeFile(oldFile.absoluteFilePath()))
            group.second->addFile(newFile.absoluteFilePath());

    return true;
}

// ----------------------------------------------------------------------------
bool ReplayManager::saveReplayWithDefaultSettings(rfcommon::Session* session)
{
    auto map = session->tryGetMappingInfo();
    auto mdata = session->tryGetMetaData();
    auto type = mdata ? mdata->type() : rfcommon::MetaData::GAME;

    // Determine target directory based on type
    QDir dir = type == rfcommon::MetaData::TRAINING ?
            defaultTrainingSessionSourceDirectory() :
            defaultGameSessionSourceDirectory();

    // Determine format string based on type
    const char* formatStr = type == rfcommon::MetaData::GAME ?
                "%date - %format (%set) - %p1name (%p1char) vs %p2name (%p2char) Game %game" :
                "%date - Training - %p1name (%p1char) on %p2char Session %session";

    if (dir.exists() == false)
        dir.mkpath(".");
    QFileInfo fileInfo(dir, composeFileName(map, mdata, formatStr));
    QByteArray ba = fileInfo.absoluteFilePath().toUtf8();
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
