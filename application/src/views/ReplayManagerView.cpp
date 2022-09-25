#include "application/models/ReplayListModel.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/views/ReplayGroupListView.hpp"
#include "application/views/ReplayListView.hpp"
#include "application/views/ReplayManagerView.hpp"

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
}

// ----------------------------------------------------------------------------
ReplayManagerView::~ReplayManagerView()
{
}

}
