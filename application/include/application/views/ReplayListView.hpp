#pragma once

#include <QTreeView>

class QItemSelection;

namespace rfapp {

class ReplayListModel;

class ReplayListView : public QTreeView
{
    Q_OBJECT

public:
    explicit ReplayListView(QWidget* parent=nullptr);
    ~ReplayListView();

    QSize sizeHint() const override;

private slots:
    void onItemRightClicked(const QPoint& pos);
    void onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteKeyPressed();

private:
};

}
