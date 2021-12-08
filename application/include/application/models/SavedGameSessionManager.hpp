#pragma once

#include "application/listeners/SavedGameSessionGroupListener.hpp"
#include "application/models/SavedGameSessionGroup.hpp"
#include "application/models/ConfigAccessor.hpp"
#include "application/Util.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <QString>
#include <QDir>
#include <unordered_map>
#include <memory>

namespace rfapp {

class ReplayManagerListener;

class ReplayManager : public ConfigAccessor
                    , public ReplayGroupListener
{
public:
    ReplayManager(Config* config);

    /*!
     * \brief This is the location where recordings are saved automatically.
     * Should always exist.
     *
     * The main window has some logic that forces the user to set a default
     * location before anything else can happen.
     */
    QDir defaultRecordingSourceDirectory() const;

    void setDefaultRecordingSourceDirectory(const QDir& path);

    bool addReplaySource(const QString& name, const QDir& path);
    bool changeReplaySourceName(const QString& oldName, const QString& newName);
    bool changeReplaySourcePath(const QString& name, const QDir& newPath);
    bool removeReplaySource(const QString& name);

    int replaySourcesCount() const;
    QString replaySourceName(int idx) const;
    QDir replaySourcePath(int idx) const;

    QString findFreeGroupName(const QString& name);
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
    ReplayGroup* replayGroup(int idx) const;
    int replayGroupsCount() const;

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets the "all" group.
     * \return The "all" group.
     */
    ReplayGroup* allReplayGroup() const;

    bool addVideoSource(const QString& name, const QDir& path);
    bool changeVideoSourceName(const QString& oldName, const QString& newName);
    bool changeVideoSourcePath(const QString& name, const QDir& newPath);
    bool removeVideoSource(const QString& name);

    int videoSourcesCount() const;
    QString videoSourceName(int idx) const;
    QDir videoSourcePath(int idx) const;

    rfcommon::ListenerDispatcher<ReplayManagerListener> dispatcher;

private:
    void scanForRecordings();

    void onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& name) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& name) override;

private:
    QHash<QString, QDir> replayDirectories_;
    QHash<QString, QDir> videoDirectories_;
    std::unordered_map<std::string, std::unique_ptr<ReplayGroup>> groups_;
};

}
