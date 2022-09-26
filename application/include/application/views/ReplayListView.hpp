#pragma once

#include <QWidget>

class QTreeView;
class QItemSelection;

namespace rfapp {

class ReplayListModel;

class ReplayListView
        : public QWidget
{
    Q_OBJECT

public:
    explicit ReplayListView(QWidget* parent=nullptr);
    ~ReplayListView();

private slots:
    void onItemRightClicked(const QPoint& pos);
    void onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteKeyPressed();

private:
    std::unique_ptr<ReplayListModel> replayListModel_;
    QTreeView* treeView_;
};

}
