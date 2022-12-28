#pragma once

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

class CategoryTabsView : public QTabWidget
{
    Q_OBJECT
public:
    explicit CategoryTabsView(
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
