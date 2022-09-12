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

    virtual void onReplayManagerGamePathAdded(const QDir& path) = 0;
    virtual void onReplayManagerGamePathRemoved(const QDir& path) = 0;

    virtual void onReplayManagerVideoPathAdded(const QDir& path) = 0;
    virtual void onReplayManagerVideoPathRemoved(const QDir& path) = 0;
};

}
