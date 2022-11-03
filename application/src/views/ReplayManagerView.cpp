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
    replayListModel_->setReplayGroup(replayManager_->allReplayGroup());
    replayListSortFilterModel_->setSourceModel(replayListModel_.get());
    replayListSortFilterModel_->setDynamicSortFilter(true);
    replayListSortFilterModel_->sort(0);
    replayListView_->setModel(replayListSortFilterModel_.get());

    replayListView_->expandAll();
    for (int i = 0; i != replayListSortFilterModel_->columnCount(); ++i)
        replayListView_->resizeColumnToContents(i);
    replayListView_->collapseAll();

    if (replayListSortFilterModel_->rowCount() > 0)
        replayListView_->expand(replayListSortFilterModel_->index(0, 0));
    if (replayListSortFilterModel_->rowCount() > 1)
        replayListView_->expand(replayListSortFilterModel_->index(1, 0));

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

    connect(searchBox, &ReplaySearchBox::searchTextChanged, this, &ReplayManagerView::searchTextChanged);
}

// ----------------------------------------------------------------------------
ReplayManagerView::~ReplayManagerView()
{
}

// ----------------------------------------------------------------------------
void ReplayManagerView::searchTextChanged(int type, const QStringList& text)
{
    if (replayListSortFilterModel_->filtersCleared() && text.size() > 0)
    {
        storeExpandedStates_.clear();
        for (int row = 0; row != replayListSortFilterModel_->rowCount(); ++row)
        {
            const QModelIndex idx = replayListSortFilterModel_->index(row, 0);
            storeExpandedStates_.push_back(replayListView_->isExpanded(idx));
        }
    }

    switch (type)
    {
        case ReplaySearchBox::GENERIC: replayListSortFilterModel_->setGenericSearchTerms(text); break;
    }

    if (replayListSortFilterModel_->filtersCleared())
    {
        for (int row = 0; row != replayListSortFilterModel_->rowCount(); ++row)
        {
            const QModelIndex idx = replayListSortFilterModel_->index(row, 0);
            replayListView_->setExpanded(idx, storeExpandedStates_[row]);
        }
    }
    else
    {
        replayListView_->expandAll();
    }
}

}
