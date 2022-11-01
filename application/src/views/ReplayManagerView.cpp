#include "application/models/ReplayListModel.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/views/ReplayGroupListView.hpp"
#include "application/views/ReplayListView.hpp"
#include "application/views/ReplayManagerView.hpp"
#include "application/models/ReplayManager.hpp"

#include <QSplitter>
#include <QVBoxLayout>

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
    , replayListModel_(new ReplayListModel)
    , replayListView_(new ReplayListView)
    , replayGroupListView_(new ReplayGroupListView)
    , pluginDockView_(new PluginDockView(replayManager, pluginManager))
{
    /*QSplitter* vSplitter = new QSplitter;
    vSplitter->addWidget(replayListView_);*/

    replayListModel_->setReplayGroup(replayManager_->allReplayGroup());
    replayListView_->setModel(replayListModel_.get());
    replayListView_->expandAll();

    for (int i = 0; i != replayListModel_->columnCount(); ++i)
        replayListView_->resizeColumnToContents(i);

    QSplitter* vSplitter = new QSplitter(Qt::Vertical);
    vSplitter->addWidget(replayGroupListView_);
    vSplitter->addWidget(replayListView_);
    vSplitter->setStretchFactor(0, 0);
    vSplitter->setStretchFactor(1, 1);

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
