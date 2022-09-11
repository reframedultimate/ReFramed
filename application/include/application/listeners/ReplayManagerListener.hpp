#pragma once

#include <QString>
#include <QDir>

namespace rfapp {

class ReplayGroup;
class ReplayManagerListener
{
public:
    virtual void onReplayManagerDefaultGamePathChanged(const QDir& path) = 0;

    virtual void onReplayManagerGroupAdded(ReplayGroup* group) = 0;
    virtual void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) = 0;
    virtual void onReplayManagerGroupRemoved(ReplayGroup* group) = 0;

    virtual void onReplayManagerGamePathAdded(const QString& name, const QDir& path) = 0;
    virtual void onReplayManagerGamePathNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onReplayManagerGamePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) = 0;
    virtual void onReplayManagerGamePathRemoved(const QString& name) = 0;

    virtual void onReplayManagerVideoPathAdded(const QString& name, const QDir& path) = 0;
    virtual void onReplayManagerVideoPathNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onReplayManagerVideoPathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) = 0;
    virtual void onReplayManagerVideoPathRemoved(const QString& name) = 0;
};

}
