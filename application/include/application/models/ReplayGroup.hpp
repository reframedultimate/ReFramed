#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include <QString>
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

    const QSet<QString>& fileNames() const;

    void addFile(const QString& fileName);
    bool removeFile(const QString& fileName);
    void removeAllFiles();
    bool isInGroup(const QString& fileName) const;

    rfcommon::ListenerDispatcher<ReplayGroupListener> dispatcher;

private:
    // Only the replay manager is allowed to change names of
    // groups because the hash table keys must remain in sync with the name
    // stored in the group object
    friend class ReplayManager;
    void setName(const QString& name);

private:
    QString name_;
    QSet<QString> fileList_;
};

}
