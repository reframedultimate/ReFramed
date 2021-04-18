#pragma once

#include "application/listeners/ListenerDispatcher.hpp"
#include "application/listeners/RecordingGroupListener.hpp"
#include "application/models/RecordingGroup.hpp"
#include "application/models/ConfigAccessor.hpp"
#include <QString>
#include <QDir>
#include <unordered_map>
#include <memory>

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
namespace std {
    template<> struct hash<QString> {
        std::size_t operator()(const QString& s) const noexcept {
            return (size_t)qHash(s);
        }
    };
}
#endif

namespace uh {

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

    ListenerDispatcher<RecordingManagerListener> dispatcher;

private:
    void scanForRecordings();

    void onRecordingGroupNameChanged(const QString& name) override;
    void onRecordingGroupFileAdded(const QFileInfo& name) override;
    void onRecordingGroupFileRemoved(const QFileInfo& name) override;

private:
    Settings* settings_;
    QHash<QString, QDir> recordingSources_;
    QHash<QString, QDir> videoDirectories_;
    std::unordered_map<QString, std::unique_ptr<RecordingGroup>> recordingGroups_;
};

}
