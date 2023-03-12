#pragma once

#include "application/models/ConfigAccessor.hpp"
#include <QWidget>
#include <memory>

class QItemSelection;
class QListWidgetItem;

namespace rfcommon {
    class ReplayMetadataCache;
}

namespace rfapp {

class CollapsibleSplitter;
class PlayerDetails;
class PluginDockView;
class PluginManager;
class ReplayGroupListView;
class ReplayListModel;
class ReplayListSortFilterModel;
class ReplayListView;
class ReplayMetadataCache;
class ReplayManager;
class MotionLabelsManager;

/*!
 * \brief Contains the list of replay groups, the list of replays in each group,
 * and the dockable plugins view. This is where the user organizes and views all
 * of their replay files.
 */
class ReplayManagerView
        : public QWidget
        , public ConfigAccessor
{
    Q_OBJECT

public:
    explicit ReplayManagerView(
            Config* config,
            ReplayManager* replayManager,
            PluginManager* pluginManager,
            PlayerDetails* playerDetails,
            MotionLabelsManager* motionLabelsManager,
            QWidget* parent=nullptr);
    ~ReplayManagerView();

    void toggleSideBar();

private slots:
    void groupSelected(QListWidgetItem* current, QListWidgetItem* previous);
    void searchTextChanged(int type, const QStringList& text);
    void onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onReplayRightClicked(const QPoint& pos);
    void onGroupRightClicked(const QPoint& pos);

    void doEditMetaData(const QStringList& selectedFileNames);
    void doAssociateVideo(const QStringList& selectedFileNames);
    void doExportReplayPack(const QStringList& selectedFileNames);
    void doAddToNewGroup(const QStringList& selectedFileNames);
    void doRemoveFromGroup(const QStringList& selectedFileNames);
    void doDeleteReplays(const QStringList& selectedFileNames);

    void doCreateNewGroup();
    void doDuplicateGroup();
    void doDeleteGroup();

private:
    QStringList collectSelectedFileNames() const;

private:
    ReplayManager* replayManager_;
    PluginManager* pluginManager_;
    PlayerDetails* playerDetails_;
    MotionLabelsManager* motionLabelsManager_;

    std::unique_ptr<ReplayMetadataCache> replayMetadataCache_;
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
