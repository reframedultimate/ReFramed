#include "rfcommon/Profiler.hpp"
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
void ReplayListWidget::addReplay(const QString& appearName, const QString& fileName)
{
    PROFILE(ReplayListWidget, addReplay);

    QListWidgetItem* item = new QListWidgetItem(appearName);
    item->setData(Qt::UserRole, QVariant(fileName));
    addItem(item);
}

// ----------------------------------------------------------------------------
void ReplayListWidget::removeReplay(const QString& fileName)
{
    PROFILE(ReplayListWidget, removeReplay);

    for (int i = 0; i != count(); ++i)
        if (item(i)->data(Qt::UserRole).toString() == fileName)
        {
            delete item(i);
            break;
        }
}

// ----------------------------------------------------------------------------
QString ReplayListWidget::itemFileName(QListWidgetItem* item) const
{
    PROFILE(ReplayListWidget, itemMatchesFileName);

    return item->data(Qt::UserRole).toString();
}

// ----------------------------------------------------------------------------
QVector<QString> ReplayListWidget::selectedReplayFileNames() const
{
    PROFILE(ReplayListWidget, selectedReplayFilePaths);

    QVector<QString> fileNames;
    for (const auto& item : selectedItems())
        fileNames.push_back(item->data(Qt::UserRole).toString());
    return fileNames;
}

// ----------------------------------------------------------------------------
QStringList ReplayListWidget::mimeTypes() const
{
    PROFILE(ReplayListWidget, mimeTypes);

    QStringList types;
    types << "application/x-reframed-rfr";
    return types;
}

// ----------------------------------------------------------------------------
QMimeData* ReplayListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
    PROFILE(ReplayListWidget, mimeData);

    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const auto& item : items)
        stream << item->data(Qt::UserRole).toString();

    mimeData->setData("application/x-reframed-rfr", encodedData);
    return mimeData;
}

}
