#pragma once

#include "application/listeners/SavedGameSessionGroupListener.hpp"
#include "application/models/SavedGameSessionGroup.hpp"
#include "application/models/ConfigAccessor.hpp"
#include "application/Util.hpp"
#include "uh/ListenerDispatcher.hpp"
#include <QString>
#include <QDir>
#include <unordered_map>
#include <memory>

namespace uhapp {

class SavedGameSessionManagerListener;

class SavedGameSessionManager : public ConfigAccessor
                              , public SavedGameSessionGroupListener
{
public:
    SavedGameSessionManager(Config* config);

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
    const QHash<QString, QDir>& savedGameSessionSources() const;

    bool addSavedGameSessionSource(const QString& name, const QDir& path);
    bool changeSavedGameSessionSourceName(const QString& oldName, const QString& newName);
    bool removeSavedGameSessionSource(const QString& name);

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
    SavedGameSessionGroup* savedGameSessionGroup(const QString& name) const;

    /*!
     * \brief Individual recordings can be organized into named groups by the
     * user. Gets the "all" group.
     * \return The "all" group.
     */
    SavedGameSessionGroup* allSavedGameSessionGroup() const;

    const std::unordered_map<std::string, std::unique_ptr<SavedGameSessionGroup>>& savedGameSessionGroups() const;

    bool addSavedGameSessionGroup(const QString& name);
    bool renameSavedGameSessionGroup(const QString& oldName, const QString& newName);
    bool removeSavedGameSessionGroup(const QString& name);

    uh::ListenerDispatcher<SavedGameSessionManagerListener> dispatcher;

private:
    void scanForRecordings();

    void onSavedGameSessionGroupFileAdded(SavedGameSessionGroup* group, const QFileInfo& name) override;
    void onSavedGameSessionGroupFileRemoved(SavedGameSessionGroup* group, const QFileInfo& name) override;

private:
    QHash<QString, QDir> savedGameSessionDirectories_;
    QHash<QString, QDir> videoDirectories_;
    std::unordered_map<std::string, std::unique_ptr<SavedGameSessionGroup>> groups_;
};

}
