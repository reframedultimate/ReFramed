#include "application/views/ReplayGroupListView.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayGroupListView::ReplayGroupListView(QWidget* parent)
    : QListWidget(parent)
{
    addItem("All");
    addItem("Test");
}

// ----------------------------------------------------------------------------
ReplayGroupListView::~ReplayGroupListView()
{}

}
