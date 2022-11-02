#include "application/models/ReplayListModel.hpp"
#include "application/models/ReplayListSortFilterModel.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/widgets/ReplaySearchBox.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/views/ReplayGroupListView.hpp"
#include "application/views/ReplayListView.hpp"
#include "application/views/ReplayManagerView.hpp"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayManagerView::ReplayManagerView(
        ReplayManager* replayManager,
        PluginManager* pluginManager,
        UserMotionLabelsManager* userMotionLabelsManager,
        rfcommon::Hash40Strings* hash40Strings,
        QWidget* parent)
    : QWidget(parent)
    , replayManager_(replayManager)
    , pluginManager_(pluginManager)
    , userMotionLabelsManager_(userMotionLabelsManager)
    , hash40Strings_(hash40Strings)
    , replayListModel_(new ReplayListModel(replayManager))
    , replayListSortFilterModel_(new ReplayListSortFilterModel)
    , replayListView_(new ReplayListView)
    , replayGroupListView_(new ReplayGroupListView)
    , pluginDockView_(new PluginDockView(replayManager, pluginManager))
{
    /*QSplitter* vSplitter = new QSplitter;
    vSplitter->addWidget(replayListView_);*/

    replayListModel_->setReplayGroup(replayManager_->allReplayGroup());
    replayListSortFilterModel_->setSourceModel(replayListModel_.get());
    replayListSortFilterModel_->setDynamicSortFilter(true);
    replayListSortFilterModel_->sort(0);
    replayListView_->setModel(replayListSortFilterModel_.get());
    //replayListView_->setSortingEnabled(true);
    //replayListView_->sortByColumn(ReplayListModel::GameNumber, Qt::DescendingOrder);
    replayListView_->expandAll();

    for (int i = 0; i != replayListModel_->columnCount(); ++i)
        replayListView_->resizeColumnToContents(i);

    replayListView_->collapseAll();
    replayListView_->expand(replayListModel_->index(0, 0));

    ReplaySearchBox* searchBox = new ReplaySearchBox;
    QLabel* searchBoxLabel = new QLabel("Filters:");
    QHBoxLayout* searchBoxLayout = new QHBoxLayout;
    searchBoxLayout->addWidget(searchBoxLabel);
    searchBoxLayout->addWidget(searchBox);
    searchBoxLayout->setContentsMargins(QMargins(0, 6, 0, 6));
    QWidget* searchBoxContainer = new QWidget;
    searchBoxContainer->setLayout(searchBoxLayout);

    QSplitter* vSplitter = new QSplitter(Qt::Vertical);
    vSplitter->addWidget(replayGroupListView_);
    vSplitter->addWidget(searchBoxContainer);
    vSplitter->addWidget(replayListView_);
    vSplitter->setStretchFactor(0, 0);
    vSplitter->setStretchFactor(1, 0);
    vSplitter->setStretchFactor(2, 1);
    vSplitter->setCollapsible(0, false);
    vSplitter->setCollapsible(1, false);
    vSplitter->setCollapsible(2, false);

    searchBoxContainer->setFixedHeight(searchBoxContainer->sizeHint().height());

    QSplitter* hSplitter = new QSplitter(Qt::Horizontal);
    hSplitter->addWidget(vSplitter);
    hSplitter->addWidget(pluginDockView_);
    hSplitter->setStretchFactor(0, 0);
    hSplitter->setStretchFactor(1, 1);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(hSplitter);

    setLayout(l);
}

// ----------------------------------------------------------------------------
ReplayManagerView::~ReplayManagerView()
{
}

}
