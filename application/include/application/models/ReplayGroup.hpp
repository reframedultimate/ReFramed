#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include <QString>
#include <QFileInfo>
#include <QSet>

namespace rfapp {

class ReplayGroupListener;
class ReplayManager;

class ReplayGroup
{
public:
    ReplayGroup(const QString& name);

    /*!
     * \brief The name of this group. It is unique among all groups.
     */
    const QString& name() const;

    const QSet<QFileInfo>& absFilePathList() const;

    void addFile(const QFileInfo& absPathToFile);
    bool removeFile(const QFileInfo& absPathToFile);
    void removeAllFiles();
    bool exists(const QFileInfo& absPathToFile) const;

    rfcommon::ListenerDispatcher<ReplayGroupListener> dispatcher;

private:
    // Only the replay manager is allowed to change names of
    // groups because the hash table keys must remain in sync with the name
    // stored in the group object
    friend class ReplayManager;
    void setName(const QString& name);

private:
    QString name_;
    QSet<QFileInfo> fileList_;
};

}
