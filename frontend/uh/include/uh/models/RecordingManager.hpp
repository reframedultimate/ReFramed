#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include "uh/listeners/RecordingGroupListener.hpp"
#include "uh/models/RecordingGroup.hpp"
#include <QString>
#include <QDir>
#include <unordered_map>
#include <memory>

namespace uh {

class RecordingManagerListener;
class Settings;

class RecordingManager : public RecordingGroupListener
{
public:
    RecordingManager(Settings* settings);

    /*!
     * \brief This is the location where recordings are saved automatically.
     * Should always exist.
     *
     * The main window has some logic that forces the user to set a default
     * location before anything else can happen.
     */
    const QDir& defaultRecordingSourceDirectory() const;

    /*!
     * \brief Returns a list of paths to directories in which the recording
     * manager can find and load recordings.
     * \note The default recording location is also part of this list.
     */
    const QVector<QDir>& recordingSourceDirectories() const;

    /*!
     * \brief Returns a list of paths to directories in which the recording
     * manager can find and load videos that are associated with a recording.
     * \return
     */
    const QVector<QDir>& videoSourceDirectories();

    RecordingGroup* allRecordingGroup();

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets all groups.
     * \return A list of groups.
     */
    const std::unordered_map<QString, std::unique_ptr<RecordingGroup>>& recordingGroups() const;

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets a specific group by name.
     * \param The name of the group to get
     * \return A pointer to the group if it was found, or nullptr if it was not
     * found.
     */
    RecordingGroup* recordingGroup(const QString& name);

    RecordingGroup* getOrCreateRecordingGroup(const QString& name);

    void setDefaultRecordingSourceDirectory(const QDir& path);

    ListenerDispatcher<RecordingManagerListener> dispatcher;

private:
    void scanForRecordings();

    void onRecordingGroupNameChanged(const QString& name) override;
    void onRecordingGroupFileAdded(const QDir& name) override;
    void onRecordingGroupFileRemoved(const QDir& name) override;

private:
    Settings* settings_;
    QVector<QDir> recordingDirectories_;
    QVector<QDir> videoDirectories_;
    std::unordered_map<QString, std::unique_ptr<RecordingGroup>> recordingGroups_;
};

}
