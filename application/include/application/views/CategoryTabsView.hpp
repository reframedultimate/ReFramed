#pragma once

#include "application/models/ConfigAccessor.hpp"
#include <QTabWidget>

namespace rfcommon {
    class Hash40Strings;
}

namespace rfapp {

class ActiveSessionManager;
class ActiveSessionView;
class PlayerDetails;
class PluginManager;
class ReplayManager;
class ReplayManagerView;
class UserMotionLabelsManager;

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
            UserMotionLabelsManager* userMotionLabelsManager,
            rfcommon::Hash40Strings* hash40Strings,
            QWidget* parent=nullptr);
    ~CategoryTabsView();

private slots:
    void onTabBarClicked(int index);

private:
    ReplayManagerView* replayManagerView_;
    ActiveSessionView* activeSessionView_;
};

}
