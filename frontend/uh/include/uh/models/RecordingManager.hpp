#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include "uh/listeners/RecordingGroupListener.hpp"
#include "uh/models/RecordingGroup.hpp"
#include <QString>
#include <QDir>

namespace uh {

class RecordingManagerListener;

class RecordingManager : public RecordingGroupListener
{
public:
    /*!
     * \brief Returns a list of paths to directories in which the recording
     * manager can find and load recordings.
     * \return
     */
    const QVector<QDir>& recordingSourceDirectories() const;

    /*!
     * \brief Returns a list of paths to directories in which the recording
     * manager can find and load videos that are associated with a recording.
     * \return
     */
    const QVector<QDir>& videoSourceDirectories();

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets all groups.
     * \return A list of groups.
     */
    const QVector<RecordingGroup>& recordingGroups() const;

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets a specific group by name.
     * \param The name of the group to get
     * \return A pointer to the group if it was found, or nullptr if it was not
     * found.
     */
    RecordingGroup* recordingGroup(const QString& name);

    RecordingGroup* getOrCreateRecordingGroup(const QString& name);

    void rescanForRecordings();

    void setDefaultRecordingLocation(const QDir& path);

    ListenerDispatcher<RecordingManagerListener> dispatcher;

private:
    void onRecordingGroupNameChanged(const QString& name) override;
    void onRecordingGroupFileAdded(const QDir& name) override;
    void onRecordingGroupFileRemoved(const QDir& name) override;

private:
    QDir defaultRecordingLocation_;
    QVector<QDir> recordingDirectories_;
    QVector<QDir> videoDirectories_;
    QVector<RecordingGroup> recordingGroups_;
};

}
