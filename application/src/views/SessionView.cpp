#include "application/ui_SessionView.h"
#include "application/views/SessionView.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/Session.hpp"

#include <QTreeWidgetItem>

namespace rfapp {

// ----------------------------------------------------------------------------
SessionView::SessionView(PluginManager* pluginManager, QWidget* parent)
    : QWidget(parent)
    , pluginManager_(pluginManager)
    , ui_(new Ui::SessionView)
{
    ui_->setupUi(this);

    while (ui_->tabWidget->count())
        ui_->tabWidget->removeTab(0);
    for (const auto& name : {
        "Damage vs Time Plot",
        "XY Positions Plot",
        "Frame Data List",
        "Decision Graph",
        "Pikachu BridgeLab",
        "Statistics"
    }) {
        rfcommon::RealtimePlugin* plugin = pluginManager_->createRealtimeModel(name);
        if (plugin == nullptr)
            continue;
        QWidget* view = plugin->createView();
        if (view == nullptr)
        {
            pluginManager_->destroyModel(plugin);
            continue;
        }

        plugins_.push_back({ plugin, view, name });
    }

    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    QMetaObject::invokeMethod(this, "addPlotsToUI", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
void SessionView::addPlotsToUI()
{
    for (const auto& data : plugins_)
        ui_->tabWidget->addTab(data.view, data.name);
}

// ----------------------------------------------------------------------------
SessionView::~SessionView()
{
    for (const auto& data : plugins_)
    {
        data.view->setParent(nullptr);
        data.plugin->destroyView(data.view);
        pluginManager_->destroyModel(data.plugin);
    }
    
    delete ui_;
}

// ----------------------------------------------------------------------------
void SessionView::showDamagePlot()
{
    ui_->tabWidget->setCurrentWidget(ui_->tab_damage_vs_time);
}

// ----------------------------------------------------------------------------
void SessionView::setSavedGameSession(rfcommon::Session* session)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionLoaded(session);
}

// ----------------------------------------------------------------------------
void SessionView::clearSavedGameSession(rfcommon::Session* session)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionUnloaded(session);
}

// ----------------------------------------------------------------------------
void SessionView::setSavedGameSessionSet(rfcommon::Session** sessions, int count)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionSetLoaded(sessions, count);
}

// ----------------------------------------------------------------------------
void SessionView::clearSavedGameSessionSet(rfcommon::Session** sessions, int count)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionSetUnloaded(sessions, count);
}

}
