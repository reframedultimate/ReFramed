#include "application/models/ReplayListModel.hpp"
#include "application/views/ReplayListView.hpp"

#include "rfcommon/Profiler.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayListView::ReplayListView(QWidget* parent)
    : QTreeView(parent)
{
    setDragDropMode(QTreeView::DragOnly);
    setSelectionMode(QTreeView::ExtendedSelection);
}

// ----------------------------------------------------------------------------
ReplayListView::~ReplayListView()
{
}

// ----------------------------------------------------------------------------
QSize ReplayListView::sizeHint() const
{
    return QSize(900, 100);
}

// ----------------------------------------------------------------------------
void ReplayListView::onDeleteKeyPressed()
{

}

}
