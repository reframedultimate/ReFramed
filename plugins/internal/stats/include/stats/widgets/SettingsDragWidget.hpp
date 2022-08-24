#pragma once

#include "stats/StatType.hpp"
#include <QFrame>

class SettingsDragWidget : public QFrame
{
    Q_OBJECT
public:
    explicit SettingsDragWidget(QWidget* parent=nullptr);

    void addStat(StatType type);
    void removeStat(StatType type);

signals:
    void statAdded(int insertIndex, StatType type);
    void statRemoved(StatType type);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
};
