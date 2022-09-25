#pragma once

#include "application/listeners/ReplayGroupListener.hpp"
#include <QWidget>

class QTreeView;
class QItemSelection;

namespace rfapp {

class ReplayGroup;
class ReplayListModel;

class ReplayListView
        : public QWidget
        , public ReplayGroupListener
{
    Q_OBJECT

public:
    explicit ReplayListView(QWidget* parent=nullptr);
    ~ReplayListView();

    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setReplayGroup(ReplayGroup* group);
    void clearReplayGroup(ReplayGroup* group);

private slots:
    void onItemRightClicked(const QPoint& pos);
    void onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteKeyPressed();

private:
    void onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName) override;

private:
    std::unique_ptr<ReplayListModel> replayListModel_;
    QTreeView* treeView_;
    ReplayGroup* currentGroup_ = nullptr;
};

}
