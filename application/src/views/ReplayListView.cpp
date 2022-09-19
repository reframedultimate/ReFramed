#include "rfcommon/Profiler.hpp"
#include "application/views/ReplayListView.hpp"

#include <QFileInfo>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

namespace rfapp {

namespace {

class ReplayListModel : public QObject
{
public:
};

}

// ----------------------------------------------------------------------------
ReplayListView::ReplayListView(QWidget* parent)
    : QTreeView(parent)
{
    setDragDropMode(DragOnly);
    setSelectionMode(ExtendedSelection);
}

// ----------------------------------------------------------------------------
ReplayListView::~ReplayListView()
{
}

// ----------------------------------------------------------------------------
void ReplayListView::addReplay(const QString& appearName, const QString& fileName)
{
    PROFILE(ReplayListView, addReplay);
}

// ----------------------------------------------------------------------------
void ReplayListView::removeReplay(const QString& fileName)
{
    PROFILE(ReplayListView, removeReplay);
}

// ----------------------------------------------------------------------------
void ReplayListView::clear()
{
    PROFILE(ReplayListView, clear);
}

// ----------------------------------------------------------------------------
QVector<QString> ReplayListView::selectedReplayFileNames() const
{
    PROFILE(ReplayListView, selectedReplayFilePaths);

    QVector<QString> fileNames;
    return fileNames;
}

}
