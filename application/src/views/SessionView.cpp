#include "application/ui_SessionView.h"
#include "application/views/SessionView.hpp"
#include "application/views/SessionDataView.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/PlayerState.hpp"

#include <QTreeWidgetItem>

namespace rfapp {

// ----------------------------------------------------------------------------
SessionView::SessionView(PluginManager* pluginManager, QWidget* parent)
    : QWidget(parent)
    , pluginManager_(pluginManager)
    , ui_(new Ui::SessionView)
    , sessionDataView_(new SessionDataView)
{
    ui_->setupUi(this);

    damageTimePlugin_ = pluginManager_->createRealtimeModel("Damage vs Time Plot");
    xyPositionsPlugin_ = pluginManager_->createRealtimeModel("XY Positions Plot");

    if (damageTimePlugin_)
        damageTimePlot_ = damageTimePlugin_->createView();
    if (xyPositionsPlugin_)
        xyPositionPlot_ = xyPositionsPlugin_->createView();

    QVBoxLayout* dataLayout = new QVBoxLayout;
    dataLayout->addWidget(sessionDataView_);
    ui_->tab_data->setLayout(dataLayout);

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
}

// ----------------------------------------------------------------------------
SessionView::~SessionView()
{
    if (xyPositionPlot_)
    {
        xyPositionPlot_->setParent(nullptr);
        xyPositionsPlugin_->destroyView(xyPositionPlot_);
        pluginManager_->destroyModel(xyPositionsPlugin_);
    }

    if (damageTimePlot_)
    {
        damageTimePlot_->setParent(nullptr);
        damageTimePlugin_->destroyView(damageTimePlot_);
        pluginManager_->destroyModel(damageTimePlugin_);
    }

    delete ui_;
}

// ----------------------------------------------------------------------------
void SessionView::showDamagePlot()
{
    ui_->tabWidget->setCurrentWidget(ui_->tab_damage_vs_time);
}

// ----------------------------------------------------------------------------
void SessionView::setSession(rfcommon::Session* session)
{
    sessionDataView_->setSession(session);
}

// ----------------------------------------------------------------------------
void SessionView::clear()
{
    sessionDataView_->clear();
}

}
