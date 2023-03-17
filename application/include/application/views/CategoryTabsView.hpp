#pragma once

#include "application/models/ConfigAccessor.hpp"
#include <QTabWidget>

namespace rfapp {

class ActiveSessionManager;
class ActiveSessionView;
class PlayerDetails;
class PluginManager;
class ReplayManager;
class ReplayManagerView;
class MotionLabelsManager;

class CategoryTabsView
        : public QTabWidget
        , public ConfigAccessor
{
    Q_OBJECT
public:
    explicit CategoryTabsView(
            Config* config,
            ReplayManager* replayManager,
            PluginManager* pluginManager,
            ActiveSessionManager* activeSessionManager,
            PlayerDetails* playerDetails,
            MotionLabelsManager* motionLabelsManager,
            QWidget* parent=nullptr);
    ~CategoryTabsView();

private slots:
    void onTabBarClicked(int index);

private:
    ReplayManagerView* replayManagerView_;
    ActiveSessionView* activeSessionView_;
};

}
