#pragma once

#include "uh/ListenerDispatcher.hpp"
#include <QString>
#include <QFileInfo>
#include <QSet>

namespace uhapp {

class RecordingGroupListener;

class RecordingGroup
{
public:
    RecordingGroup(const QString& name);

    /*!
     * \brief The name of this group. It is unique among all groups.
     */
    const QString& name() const;

    const QSet<QFileInfo>& absFilePathList() const;

    void setName(const QString& name);
    void addFile(const QFileInfo& absPathToFile);
    bool removeFile(const QFileInfo& absPathToFile);
    void removeAllFiles();

    uh::ListenerDispatcher<RecordingGroupListener> dispatcher;

private:
    RecordingGroup() {}
    friend class QVector<RecordingGroup>;

private:
    QString name_;
    QSet<QFileInfo> fileList_;
};

}

