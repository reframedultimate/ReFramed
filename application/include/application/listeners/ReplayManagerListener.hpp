#pragma once

#include <QString>
#include <QDir>

namespace rfapp {

class ReplayGroup;
class ReplayManagerListener
{
public:
    virtual void onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path) = 0;

    virtual void onReplayManagerGroupAdded(ReplayGroup* group) = 0;
    virtual void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) = 0;
    virtual void onReplayManagerGroupRemoved(ReplayGroup* group) = 0;

    virtual void onReplayManagerReplaySourceAdded(const QString& name, const QDir& path) = 0;
    virtual void onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) = 0;
    virtual void onReplayManagerReplaySourceRemoved(const QString& name) = 0;

    virtual void onReplayManagerVideoSourceAdded(const QString& name, const QDir& path) = 0;
    virtual void onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) = 0;
    virtual void onReplayManagerVideoSourceRemoved(const QString& name) = 0;
};

}
