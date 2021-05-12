#include "application/views/RecordingListWidget.hpp"

#include <QFileInfo>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

namespace uhapp {

// ----------------------------------------------------------------------------
RecordingListWidget::RecordingListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setDragDropMode(DragOnly);
    setSelectionMode(ExtendedSelection);
}

// ----------------------------------------------------------------------------
RecordingListWidget::~RecordingListWidget()
{

}

// ----------------------------------------------------------------------------
void RecordingListWidget::addRecordingFileName(const QFileInfo& absPathToFile)
{
    QListWidgetItem* item = new QListWidgetItem(absPathToFile.completeBaseName());
    item->setData(Qt::UserRole, QVariant(absPathToFile.absoluteFilePath()));
    addItem(item);
}

// ----------------------------------------------------------------------------
void RecordingListWidget::removeRecordingFileName(const QFileInfo& absPathToFile)
{
    for (const auto& item : findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
bool RecordingListWidget::itemMatchesRecordingFileName(QListWidgetItem* item, const QFileInfo& absPathToFile)
{
    return item->data(Qt::UserRole).toString() == absPathToFile.absoluteFilePath();
}

// ----------------------------------------------------------------------------
QStringList RecordingListWidget::mimeTypes() const
{
    QStringList types;
    types << "application/x-ultimate-hindsight-uhr";
    return types;
}

// ----------------------------------------------------------------------------
QMimeData* RecordingListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const auto& item : items)
        stream << item->data(Qt::UserRole).toString();

    mimeData->setData("application/x-ultimate-hindsight-uhr", encodedData);
    return mimeData;
}

}
