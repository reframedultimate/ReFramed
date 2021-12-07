#include "application/views/SavedGameSessionListWidget.hpp"

#include <QFileInfo>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

namespace uhapp {

// ----------------------------------------------------------------------------
SavedGameSessionListWidget::SavedGameSessionListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setDragDropMode(DragOnly);
    setSelectionMode(ExtendedSelection);
}

// ----------------------------------------------------------------------------
SavedGameSessionListWidget::~SavedGameSessionListWidget()
{
}

// ----------------------------------------------------------------------------
void SavedGameSessionListWidget::addSavedGameSessionFileName(const QFileInfo& absPathToFile)
{
    QListWidgetItem* item = new QListWidgetItem(absPathToFile.completeBaseName());
    item->setData(Qt::UserRole, QVariant(absPathToFile.absoluteFilePath()));
    addItem(item);
}

// ----------------------------------------------------------------------------
void SavedGameSessionListWidget::removeSavedGameSessionFileName(const QFileInfo& absPathToFile)
{
    for (const auto& item : findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
bool SavedGameSessionListWidget::itemMatchesSavedGameSessionFileName(QListWidgetItem* item, const QFileInfo& absPathToFile)
{
    return item->data(Qt::UserRole).toString() == absPathToFile.absoluteFilePath();
}

// ----------------------------------------------------------------------------
QVector<QFileInfo> SavedGameSessionListWidget::selectedSavedGameSessionFilePaths() const
{
    QVector<QFileInfo> recordings;
    for (const auto& item : selectedItems())
        recordings.push_back(item->data(Qt::UserRole).toString());
    return recordings;
}

// ----------------------------------------------------------------------------
QStringList SavedGameSessionListWidget::mimeTypes() const
{
    QStringList types;
    types << "application/x-ultimate-hindsight-uhr";
    return types;
}

// ----------------------------------------------------------------------------
QMimeData* SavedGameSessionListWidget::mimeData(const QList<QListWidgetItem*> items) const
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
