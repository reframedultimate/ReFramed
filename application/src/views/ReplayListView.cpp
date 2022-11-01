#include "application/models/ReplayListModel.hpp"
#include "application/views/ReplayListView.hpp"

#include "rfcommon/Profiler.hpp"

#include <QTreeView>

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
void ReplayListView::onItemRightClicked(const QPoint& pos)
{

}

// ----------------------------------------------------------------------------
void ReplayListView::onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{

}

// ----------------------------------------------------------------------------
void ReplayListView::onDeleteKeyPressed()
{

}

}
