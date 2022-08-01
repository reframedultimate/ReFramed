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

    //damageTimePlugin_ = pluginManager_->createRealtimeModel("Damage vs Time Plot");
    // = pluginManager_->createRealtimeModel("XY Positions Plot");
    frameDataListPlugin_ = pluginManager_->createRealtimeModel("Frame Data List");
    decisionGraphPlugin_ = pluginManager_->createRealtimeModel("Decision Graph");

    if (damageTimePlugin_)
        damageTimePlot_ = damageTimePlugin_->createView();
    if (xyPositionsPlugin_)
        xyPositionPlot_ = xyPositionsPlugin_->createView();
    if (frameDataListPlugin_)
        frameDataListView_ = frameDataListPlugin_->createView();
    if (decisionGraphPlugin_)
        decisionGraphView_ = decisionGraphPlugin_->createView();

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

    if (xyPositionPlot_)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(xyPositionPlot_);
        ui_->tab_xy_positions->setLayout(layout);
    }

    if (decisionGraphView_)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(decisionGraphView_);
        ui_->tab_decision_graph->setLayout(layout);
    }
}

// ----------------------------------------------------------------------------
SessionView::~SessionView()
{
    if (decisionGraphView_)
    {
        decisionGraphView_->setParent(nullptr);
        decisionGraphPlugin_->destroyView(decisionGraphView_);
    }

    if (xyPositionPlot_)
    {
        xyPositionPlot_->setParent(nullptr);
        xyPositionsPlugin_->destroyView(xyPositionPlot_);
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
}

}
