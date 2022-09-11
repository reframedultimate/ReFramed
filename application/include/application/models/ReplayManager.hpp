#pragma once

#include "application/models/ReplayGroup.hpp"
#include "application/models/ConfigAccessor.hpp"
#include "application/Util.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/FilePathResolver.hpp"
#include <QString>
#include <QDir>
#include <unordered_map>
#include <memory>

namespace rfcommon {
    class GameNumber;
    class MappingInfo;
    class MetaData;
    class Session;
}

namespace rfapp {

class ReplayManagerListener;

class ReplayManager
    : public ConfigAccessor
    , public rfcommon::FilePathResolver
{
public:
    ReplayManager(Config* config);

    /*!
     * \brief This is the location where replays of games are saved 
     * automatically. Should always exist.
     *
     * The main window has some logic that forces the user to set a default
     * location before anything else can happen.
     */
    QDir defaultGamePath() const;

    /*!
     * \brief Changes the location where replays of games are saved.
     * \note If there are replays in the existing source directory, then
     * the path to the existing directory is added to the list of source
     * directories via addGamePath() before changing the default
     * directory. This makes sure that all replays can still be found.
     */
    void setDefaultGamePath(const QDir& path);

    /*!
     * \brief Gets the path to the default directory to store replay files
     * from training sessions.
     */
    QDir defaultTrainingPath() const;

    bool addGamePath(const QString& name, const QDir& path);
    bool changeGamePathName(const QString& oldName, const QString& newName);
    bool changeGamePath(const QString& name, const QDir& newPath);
    bool removeGamePath(const QString& name);

    int gamePathCount() const;
    QString gamePathName(int idx) const;
    QString gamePathName(const QDir& path) const;
    QDir gamePath(int idx) const;
    QDir gamePath(const QString& name) const;
    bool gamePathNameExists(const QString& name) const;
    bool gamePathExists(const QDir& path) const;

    QString findFreeGroupName(const QString& name) const;
    ReplayGroup* addReplayGroup(const QString& name);
    ReplayGroup* duplicateReplayGroup(ReplayGroup* group, const QString& newName);
    bool renameReplayGroup(ReplayGroup* group, const QString& newName);
    bool removeReplayGroup(ReplayGroup* group);

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets a specific group by name.
     * \param The name of the group to get
     * \return A pointer to the group if it was found, or nullptr if it was not
     * found.
     */
    ReplayGroup* replayGroup(const QString& name) const;

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets a specific group by index.
     * \note The "All" group cannot be retrieved by index, and replayGroupsCount()
     * excludes the "All" group as well.
     */
    ReplayGroup* replayGroup(int idx) const;

    int replayGroupsCount() const;

    /*!
     * \brief Individual replays can be organized into named groups by the
     * user. The "all" group is a special group that cannot be renamed or
     * deleted, and it contains all accessible replays.
     * \return The "all" group.
     */
    ReplayGroup* allReplayGroup() const;

    bool addVideoPath(const QString& name, const QDir& path);
    bool changeVideoPathName(const QString& oldName, const QString& newName);
    bool changeVideoPath(const QString& name, const QDir& newPath);
    bool removeVideoPath(const QString& name);

    int videoPathCount() const;
    QString videoPathName(int idx) const;
    QDir videoPath(int idx) const;
    bool videoPathNameExists(const QString& name) const;
    bool videoPathExists(const QDir& path) const;

    QString composeFileName(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata, QString formatString);
    bool findFreeSetAndGameNumbers(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata);
    bool saveReplayOver(rfcommon::Session* session, const QString& oldFileName);
    bool saveReplayWithDefaultSettings(rfcommon::Session* session);
    bool deleteReplay(const QString& fileName);

    rfcommon::ListenerDispatcher<ReplayManagerListener> dispatcher;

private:
    void scanForReplays();
    void updateAndSaveConfig();

public:
    rfcommon::String resolveGameFile(const char* fileName) const override;
    rfcommon::String resolveVideoFile(const char* fileName) const override;

private:
    QDir defaultGamePath_;
    QDir defaultTrainingPath_;
    QHash<QString, QDir> gamePaths_;
    QHash<QString, QDir> trainingPaths_;
    QHash<QString, QDir> videoPaths_;
    std::unique_ptr<ReplayGroup> allGroup_;
    std::unordered_map<std::string, std::unique_ptr<ReplayGroup>> groups_;
};

}
