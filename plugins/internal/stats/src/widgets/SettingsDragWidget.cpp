#include "rfcommon/Profiler.hpp"
#include "stats/widgets/SettingsDragWidget.hpp"
#include "stats/widgets/SettingsStatsItem.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDrag>

#include <QDebug>

// ----------------------------------------------------------------------------
SettingsDragWidget::SettingsDragWidget(QWidget* parent)
    : QFrame(parent)
{
    setAcceptDrops(true);

    setLayout(new QVBoxLayout);
}

// ----------------------------------------------------------------------------
void SettingsDragWidget::addStat(StatType type)
{
    PROFILE(SettingsDragWidget, addStat);

    layout()->addWidget(new SettingsStatsItem(type));
}

// ----------------------------------------------------------------------------
void SettingsDragWidget::removeStat(StatType type)
{
    PROFILE(SettingsDragWidget, removeStat);

    for (int i = 0; i != layout()->count(); ++i)
    {
        SettingsStatsItem* item = static_cast<SettingsStatsItem*>(layout()->itemAt(i)->widget());
        if (item->type() == type)
        {
            delete item;
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void SettingsDragWidget::dragEnterEvent(QDragEnterEvent* e)
{
    PROFILE(SettingsDragWidget, dragEnterEvent);

    if (!e->mimeData()->hasFormat("application/x-stats-settings-item"))
    {
        e->ignore();
        return;
    }

    if (e->source() == this)
    {
        e->setDropAction(Qt::MoveAction);
        e->accept();
    }
    else
    {
        e->acceptProposedAction();
    }
}

// ----------------------------------------------------------------------------
void SettingsDragWidget::dragMoveEvent(QDragMoveEvent* e)
{
    PROFILE(SettingsDragWidget, dragMoveEvent);

    if (!e->mimeData()->hasFormat("application/x-stats-settings-item"))
    {
        e->ignore();
        return;
    }

    if (e->source() == this)
    {
        e->setDropAction(Qt::MoveAction);
        e->accept();
    }
    else
    {
        e->acceptProposedAction();
    }
}

// ----------------------------------------------------------------------------
void SettingsDragWidget::dropEvent(QDropEvent* e)
{
    PROFILE(SettingsDragWidget, dropEvent);

    if (!e->mimeData()->hasFormat("application/x-stats-settings-item"))
    {
        e->ignore();
        return;
    }

    QByteArray itemData = e->mimeData()->data("application/x-stats-settings-item");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);

    StatType type;
    dataStream >> type;

    int insertIndex = 0;
    while (insertIndex != layout()->count())
    {
        QLayoutItem* item = layout()->itemAt(insertIndex);
        QRect g = item->geometry();
        int dropY = e->position().toPoint().x();
        int itemHeight = g.height();
        int itemY = g.y() + itemHeight / 2;

        if (dropY < itemY)
            break;

        insertIndex++;
    }

    static_cast<QVBoxLayout*>(layout())->insertWidget(insertIndex, new SettingsStatsItem(type));

    // If an item was dragged and dropped between the same widget, then it will exist
    // twice in the layout for a short amount of time. The following code finds the
    // previous location of the same item (which will be deleted once this function
    // returns), and is used to correct the insertion index so our SettingsModel
    // gets the correct value
    int prevLocation = 0;
    while (prevLocation != layout()->count())
    {
        SettingsStatsItem* item = static_cast<SettingsStatsItem*>(layout()->itemAt(prevLocation)->widget());
        if (type == item->type())
            break;
        prevLocation++;
    }

    if (e->source() == this)
    {
        e->setDropAction(Qt::MoveAction);
        e->accept();
    }
    else
    {
        e->acceptProposedAction();
    }

    // Correct insert index for SettingsModel
    if (prevLocation < insertIndex)
        insertIndex--;

    emit statAdded(insertIndex, type);
}

// ----------------------------------------------------------------------------
void SettingsDragWidget::mousePressEvent(QMouseEvent* e)
{
    PROFILE(SettingsDragWidget, mousePressEvent);

    QWidget* widget = childAt(e->pos());
    SettingsStatsItem* item = qobject_cast<SettingsStatsItem*>(widget);
    QLabel* item2 = qobject_cast<QLabel*>(widget);
    if (item == nullptr)
        return;

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << item->type();

    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/x-stats-settings-item", itemData);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(e->pos() - item->pos());

    item->setDragInProgress();
    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
    {
        emit statRemoved(item->type());
        delete item;
    }
    else
    {
        item->setDragCancelled();
    }
}
