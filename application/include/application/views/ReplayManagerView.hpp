#pragma once

#include <QWidget>
#include <memory>

class QItemSelection;
class QListWidgetItem;

namespace rfcommon {
    class Hash40Strings;
}

namespace rfapp {

class CollapsibleSplitter;
class PlayerDetails;
class PluginDockView;
class PluginManager;
class ReplayListModel;
class ReplayListSortFilterModel;
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
            PlayerDetails* playerDetails,
            UserMotionLabelsManager* userMotionLabelsManager,
            rfcommon::Hash40Strings* hash40Strings,
            QWidget* parent=nullptr);
    ~ReplayManagerView();

    void toggleSideBar();

private slots:
    void groupSelected(QListWidgetItem* current, QListWidgetItem* previous);
    void searchTextChanged(int type, const QStringList& text);
    void onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onReplayRightClicked(const QPoint& pos);
    void onGroupRightClicked(const QPoint& pos);

private:
    ReplayManager* replayManager_;
    PluginManager* pluginManager_;
    PlayerDetails* playerDetails_;
    UserMotionLabelsManager* userMotionLabelsManager_;
    rfcommon::Hash40Strings* hash40Strings_;

    std::unique_ptr<ReplayListModel> replayListModel_;
    std::unique_ptr<ReplayListSortFilterModel> replayListSortFilterModel_;
    ReplayListView* replayListView_;
    ReplayGroupListView* replayGroupListView_;
    PluginDockView* pluginDockView_;

    CollapsibleSplitter* hSplitter_;
    int size0_, size1_;

    QVector<bool> storeExpandedStates_;
};

}
