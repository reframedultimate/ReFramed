#pragma once

#include <QTreeView>

namespace rfapp {

class ReplayListView : public QTreeView
{
    Q_OBJECT

public:
    explicit ReplayListView(QWidget* parent=nullptr);
    ~ReplayListView();

    void addReplay(const QString& appearName, const QString& fileName);
    void removeReplay(const QString& fileName);
    void clear();
    QVector<QString> selectedReplayFileNames() const;
};

}
