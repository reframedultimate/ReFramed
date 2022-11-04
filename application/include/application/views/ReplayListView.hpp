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
    void onDeleteKeyPressed();

private:
};

}
