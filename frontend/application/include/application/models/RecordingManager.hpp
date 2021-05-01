#pragma once

#include "application/listeners/RecordingGroupListener.hpp"
#include "application/models/RecordingGroup.hpp"
#include "application/models/ConfigAccessor.hpp"
#include "application/Util.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/HashMap.hpp"
#include <QString>
#include <QDir>
#include <memory>

namespace uhapp {

class RecordingManagerListener;
class Settings;

class RecordingManager : public ConfigAccessor
                       , public RecordingGroupListener
{
public:
    RecordingManager(Config* config);

    /*!
     * \brief This is the location where recordings are saved automatically.
     * Should always exist.
     *
     * The main window has some logic that forces the user to set a default
     * location before anything else can happen.
     */
    QDir defaultRecordingSourceDirectory() const;

    void setDefaultRecordingSourceDirectory(const QDir& path);

    /*!
     * \brief Returns a list of paths to directories in which the recording
     * manager can find and load recordings.
     * \note The default recording location is also part of this list.
     */
    const QHash<QString, QDir>& recordingSources() const;

    bool addRecordingSource(const QString& name, const QDir& path);
    bool changeRecordingSourceName(const QString& oldName, const QString& newName);
    bool removeRecordingSource(const QString& name);

    /*!
     * \brief Returns a list of paths to directories in which the recording
     * manager can find and load videos that are associated with a recording.
     * \return
     */
    const QHash<QString, QDir>& videoSources();

    bool addVideoSource(const QString& name, const QDir& path);
    bool changeVideoSourceName(const QString& oldName, const QString& newName);
    bool removeVideoSource(const QString& name);

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets a specific group by name.
     * \param The name of the group to get
     * \return A pointer to the group if it was found, or nullptr if it was not
     * found.
     */
    RecordingGroup* recordingGroup(const QString& name) const;

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets all groups.
     * \return A list of groups.
     */
    RecordingGroup* allRecordingGroup() const;

    const uh::HashMap<QString, std::unique_ptr<RecordingGroup>, QStringHasher<>>& recordingGroups() const;

    bool addRecordingGroup(const QString& name);
    bool renameRecordingGroup(const QString& oldName, const QString& newName);
    bool removeRecordingGroup(const QString& name);

    uh::ListenerDispatcher<RecordingManagerListener> dispatcher;

private:
    void scanForRecordings();

    void onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& name) override;
    void onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& name) override;

private:
    Settings* settings_;
    QHash<QString, QDir> recordingSources_;
    QHash<QString, QDir> videoDirectories_;
    uh::HashMap<QString, std::unique_ptr<RecordingGroup>, QStringHasher<>> recordingGroups_;
};

}
