#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include <QString>
#include <QDir>
#include <QVector>

namespace uh {

class RecordingGroupListener;

class RecordingGroup
{
public:
    RecordingGroup(const QString& name);

    const QVector<QDir>& fileList() const;
    const QString& name() const;

    void setName(const QString& name);
    void addFile(const QDir& pathToFile);
    bool removeFile(const QDir& pathToFile);

    ListenerDispatcher<RecordingGroupListener> dispatcher;

private:
    RecordingGroup() {}
    friend class QVector<RecordingGroup>;

private:
    QString name_;
    QVector<QDir> fileList_;
};

}
