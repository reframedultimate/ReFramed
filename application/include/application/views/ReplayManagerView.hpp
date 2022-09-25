#pragma once

#include <QWidget>
#include <memory>

namespace rfcommon {
    class Hash40Strings;
}

namespace rfapp {

class PluginDockView;
class PluginManager;
class ReplayListModel;
class ReplayListView;
class ReplayGroupListView;
class ReplayManager;
class UserMotionLabelsManager;

/*!
 * \brief Contains the list of replay groups, the list of replays in each group,
 * and the dockable plugins view. This is where the user organizes and views all
 * of their replay files.
 */
class ReplayManagerView : public QWidget
{
    Q_OBJECT

public:
    explicit ReplayManagerView(
            ReplayManager* replayManager,
            PluginManager* pluginManager,
            UserMotionLabelsManager* userMotionLabelsManager,
            rfcommon::Hash40Strings* hash40Strings,
            QWidget* parent=nullptr);
    ~ReplayManagerView();

private:
    ReplayManager* replayManager_;
    PluginManager* pluginManager_;
    UserMotionLabelsManager* userMotionLabelsManager_;
    rfcommon::Hash40Strings* hash40Strings_;

    std::unique_ptr<ReplayListModel> replayListModel_;
    ReplayListView* replayListView_;
    ReplayGroupListView* replayGroupListView_;

    PluginDockView* pluginDockView_;
};

}
