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

    damageTimePlugin_ = pluginManager_->createRealtimeModel("Damage vs Time Plot");
    xyPositionsPlugin_ = pluginManager_->createRealtimeModel("XY Positions Plot");
    frameDataListPlugin_ = pluginManager_->createRealtimeModel("Frame Data List");
    decisionGraphPlugin_ = pluginManager_->createRealtimeModel("Decision Graph");
    bridgeLabPlugin_ = pluginManager_->createRealtimeModel("Pikachu BridgeLab");

    if (damageTimePlugin_)
        damageTimePlot_ = damageTimePlugin_->createView();
    if (xyPositionsPlugin_)
        xyPositionsPlot_ = xyPositionsPlugin_->createView();
    if (frameDataListPlugin_)
        frameDataListView_ = frameDataListPlugin_->createView();
    if (decisionGraphPlugin_)
        decisionGraphView_ = decisionGraphPlugin_->createView();
    if (bridgeLabPlugin_)
        bridgeLabView_ = bridgeLabPlugin_->createView();

    if (frameDataListView_)
    {
        QVBoxLayout* dataLayout = new QVBoxLayout;
        dataLayout->addWidget(frameDataListView_);
        ui_->tab_data->setLayout(dataLayout);
    }

    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    QMetaObject::invokeMethod(this, "addPlotsToUI", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
void SessionView::addPlotsToUI()
{
    if (damageTimePlot_)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(damageTimePlot_);
        ui_->tab_damage_vs_time->setLayout(layout);
    }

    if (xyPositionsPlot_)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(xyPositionsPlot_);
        ui_->tab_xy_positions->setLayout(layout);
    }

    if (decisionGraphView_)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(decisionGraphView_);
        ui_->tab_decision_graph->setLayout(layout);
    }

    if (bridgeLabView_)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(bridgeLabView_);
        ui_->tab_bridge_lab->setLayout(layout);
    }
}

// ----------------------------------------------------------------------------
SessionView::~SessionView()
{
    if (bridgeLabView_)
    {
        bridgeLabView_->setParent(nullptr);
        bridgeLabPlugin_->destroyView(bridgeLabView_);
    }

    if (decisionGraphView_)
    {
        decisionGraphView_->setParent(nullptr);
        decisionGraphPlugin_->destroyView(decisionGraphView_);
    }

    if (xyPositionsPlot_)
    {
        xyPositionsPlot_->setParent(nullptr);
        xyPositionsPlugin_->destroyView(xyPositionsPlot_);
    }

    if (damageTimePlot_)
    {
        damageTimePlot_->setParent(nullptr);
        damageTimePlugin_->destroyView(damageTimePlot_);
    }

    if (frameDataListView_)
    {
        frameDataListView_->setParent(nullptr);
        frameDataListPlugin_->destroyView(frameDataListView_);
    }

    if (bridgeLabPlugin_)
        pluginManager_->destroyModel(bridgeLabPlugin_);
    if (decisionGraphPlugin_)
        pluginManager_->destroyModel(decisionGraphPlugin_);
    if (xyPositionsPlugin_)
        pluginManager_->destroyModel(xyPositionsPlugin_);
    if (damageTimePlugin_)
        pluginManager_->destroyModel(damageTimePlugin_);
    if (frameDataListPlugin_)
        pluginManager_->destroyModel(frameDataListPlugin_);

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
    if (damageTimePlugin_)
        damageTimePlugin_->onGameSessionLoaded(session);
    if (xyPositionsPlugin_)
        xyPositionsPlugin_->onGameSessionLoaded(session);
    if (frameDataListPlugin_)
        frameDataListPlugin_->onGameSessionLoaded(session);
    if (decisionGraphPlugin_)
        decisionGraphPlugin_->onGameSessionLoaded(session);
    if (bridgeLabPlugin_)
        bridgeLabPlugin_->onGameSessionLoaded(session);
}

// ----------------------------------------------------------------------------
void SessionView::clearSavedGameSession(rfcommon::Session* session)
{
    if (damageTimePlugin_)
        damageTimePlugin_->onGameSessionUnloaded(session);
    if (xyPositionsPlugin_)
        xyPositionsPlugin_->onGameSessionUnloaded(session);
    if (frameDataListPlugin_)
        frameDataListPlugin_->onGameSessionUnloaded(session);
    if (decisionGraphPlugin_)
        decisionGraphPlugin_->onGameSessionUnloaded(session);
    if (bridgeLabPlugin_)
        bridgeLabPlugin_->onGameSessionUnloaded(session);
}

// ----------------------------------------------------------------------------
void SessionView::setSavedGameSessionSet(rfcommon::Session** sessions, int count)
{
    if (damageTimePlugin_)
        damageTimePlugin_->onGameSessionSetLoaded(sessions, count);
    if (xyPositionsPlugin_)
        xyPositionsPlugin_->onGameSessionSetLoaded(sessions, count);
    if (frameDataListPlugin_)
        frameDataListPlugin_->onGameSessionSetLoaded(sessions, count);
    if (decisionGraphPlugin_)
        decisionGraphPlugin_->onGameSessionSetLoaded(sessions, count);
    if (bridgeLabPlugin_)
        bridgeLabPlugin_->onGameSessionSetLoaded(sessions, count);
}

// ----------------------------------------------------------------------------
void SessionView::clearSavedGameSessionSet(rfcommon::Session** sessions, int count)
{
    if (damageTimePlugin_)
        damageTimePlugin_->onGameSessionSetUnloaded(sessions, count);
    if (xyPositionsPlugin_)
        xyPositionsPlugin_->onGameSessionSetUnloaded(sessions, count);
    if (frameDataListPlugin_)
        frameDataListPlugin_->onGameSessionSetUnloaded(sessions, count);
    if (decisionGraphPlugin_)
        decisionGraphPlugin_->onGameSessionSetUnloaded(sessions, count);
    if (bridgeLabPlugin_)
        bridgeLabPlugin_->onGameSessionSetUnloaded(sessions, count);
}

}
