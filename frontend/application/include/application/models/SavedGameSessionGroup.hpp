#pragma once

#include "uh/ListenerDispatcher.hpp"
#include <QString>
#include <QFileInfo>
#include <QSet>

namespace uhapp {

class SavedGameSessionGroupListener;
class SavedGameSessionManager;

class SavedGameSessionGroup
{
public:
    SavedGameSessionGroup(const QString& name);

    /*!
     * \brief The name of this group. It is unique among all groups.
     */
    const QString& name() const;

    const QSet<QFileInfo>& absFilePathList() const;

    void addFile(const QFileInfo& absPathToFile);
    bool removeFile(const QFileInfo& absPathToFile);
    void removeAllFiles();

    uh::ListenerDispatcher<SavedGameSessionGroupListener> dispatcher;

private:
    // Only the recording manager is allowed to change names of recording groups
    // because the hash table keys must remain in sync with the name stored in
    // the recording group object
    friend class RecordingManager;
    void setName(const QString& name);

private:
    QString name_;
    QSet<QFileInfo> fileList_;
};

}
