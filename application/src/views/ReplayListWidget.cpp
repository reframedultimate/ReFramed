#include "application/views/ReplayListWidget.hpp"

#include <QFileInfo>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayListWidget::ReplayListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setDragDropMode(DragOnly);
    setSelectionMode(ExtendedSelection);
}

// ----------------------------------------------------------------------------
ReplayListWidget::~ReplayListWidget()
{
}

// ----------------------------------------------------------------------------
void ReplayListWidget::addReplayFileName(const QFileInfo& absPathToFile)
{
    QListWidgetItem* item = new QListWidgetItem(absPathToFile.completeBaseName());
    item->setData(Qt::UserRole, QVariant(absPathToFile.absoluteFilePath()));
    addItem(item);
}

// ----------------------------------------------------------------------------
void ReplayListWidget::removeReplayFileName(const QFileInfo& absPathToFile)
{
    for (const auto& item : findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
bool ReplayListWidget::itemMatchesReplayFileName(QListWidgetItem* item, const QFileInfo& absPathToFile)
{
    return item->data(Qt::UserRole).toString() == absPathToFile.absoluteFilePath();
}

// ----------------------------------------------------------------------------
QVector<QFileInfo> ReplayListWidget::selectedReplayFilePaths() const
{
    QVector<QFileInfo> recordings;
    for (const auto& item : selectedItems())
        recordings.push_back(item->data(Qt::UserRole).toString());
    return recordings;
}

// ----------------------------------------------------------------------------
QStringList ReplayListWidget::mimeTypes() const
{
    QStringList types;
    types << "application/x-ultimate-hindsight-rfr";
    return types;
}

// ----------------------------------------------------------------------------
QMimeData* ReplayListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const auto& item : items)
        stream << item->data(Qt::UserRole).toString();

    mimeData->setData("application/x-ultimate-hindsight-rfr", encodedData);
    return mimeData;
}

}
