#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include <QListWidget>

namespace rfapp {

class ReplayManager;

class ReplayGroupListView
        : public QListWidget
        , public ReplayManagerListener
{
public:
    explicit ReplayGroupListView(ReplayManager* replayManager, QWidget* parent=nullptr);
    ~ReplayGroupListView();

private:
    void onReplayManagerDefaultGamePathChanged(const QDir& path) override {}

    void onReplayManagerGroupAdded(ReplayGroup* group) override;
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override;
    void onReplayManagerGroupRemoved(ReplayGroup* group) override;

    void onReplayManagerGamePathAdded(const QDir& path) override {}
    void onReplayManagerGamePathRemoved(const QDir& path) override {}

    void onReplayManagerVideoPathAdded(const QDir& path) override {}
    void onReplayManagerVideoPathRemoved(const QDir& path) override {}

private:
    ReplayManager* replayManager_;
};

}
