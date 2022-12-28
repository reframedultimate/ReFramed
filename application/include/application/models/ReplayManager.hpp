#pragma once

#include "application/models/ReplayGroup.hpp"
#include "application/models/ConfigAccessor.hpp"
#include "application/Util.hpp"

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/FilePathResolver.hpp"

#include <QString>
#include <QDir>
#include <QSet>
#include <QHash>

#include <memory>

namespace rfcommon {
    class GameNumber;
    class MappingInfo;
    class Metadata;
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
    ~ReplayManager();

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

public:
    bool addGamePath(const QDir& path);
    bool removeGamePath(const QDir& path);
    int gamePathCount() const;
    const QDir& gamePath(int idx) const;
    bool gamePathExists(const QDir& path) const;

public:
    bool addVideoPath(const QDir& path);
    bool removeVideoPath(const QDir& path);
    int videoPathCount() const;
    const QDir& videoPath(int idx) const;
    bool videoPathExists(const QDir& path) const;

public:
    QString findFreeGroupName(const QString& name) const;
    ReplayGroup* addReplayGroup(const QString& name);
    ReplayGroup* duplicateReplayGroup(ReplayGroup* group, const QString& newName);
    bool renameReplayGroup(ReplayGroup* group, const QString& newName);
    bool removeReplayGroup(ReplayGroup* group);

    int replayGroupCount() const;

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

    /*!
     * \brief Individual replays can be organized into named groups by the
     * user. The "all" group is a special group that cannot be renamed or
     * deleted, and it contains all accessible replays.
     * \return The "all" group.
     */
    ReplayGroup* allReplayGroup() const;

public:
    bool saveReplayOver(rfcommon::Session* session, const QString& oldFileName);
    bool saveReplayWithDefaultSettings(rfcommon::Session* session);
    bool deleteReplay(const QString& fileName);

    rfcommon::ListenerDispatcher<ReplayManagerListener> dispatcher;

private:
    void scanForReplays();
    void updateConfig();

public:
    rfcommon::String resolveGameFile(const char* fileName) const override;
    rfcommon::String resolveVideoFile(const char* fileName) const override;

private:
    QDir defaultGamePath_;
    QDir defaultTrainingPath_;
    QSet<QDir> gamePaths_;
    QSet<QDir> trainingPaths_;
    QSet<QDir> videoPaths_;
    QHash<QString, ReplayGroup*> groups_;
    QScopedPointer<ReplayGroup> allGroup_;
};

}
