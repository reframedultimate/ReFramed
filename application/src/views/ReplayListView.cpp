#include "application/models/ReplayListModel.hpp"
#include "application/views/ReplayListView.hpp"

#include "rfcommon/Profiler.hpp"

#include <QTreeView>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayListView::ReplayListView(QWidget* parent)
    : QWidget(parent)
    , replayListModel_(new ReplayListModel)
    , treeView_(new QTreeView)
{
    treeView_->setDragDropMode(QTreeView::DragOnly);
    treeView_->setSelectionMode(QTreeView::ExtendedSelection);
    treeView_->setModel(replayListModel_.get());
}

// ----------------------------------------------------------------------------
ReplayListView::~ReplayListView()
{
}

}
