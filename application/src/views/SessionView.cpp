#include "application/ui_SessionView.h"
#include "application/views/SessionView.hpp"
#include "application/views/SessionDataView.hpp"
#include "application/views/DamageTimePlot.hpp"
#include "application/views/XYPositionPlot.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/PlayerState.hpp"

#include <QTreeWidgetItem>

namespace rfapp {

// ----------------------------------------------------------------------------
SessionView::SessionView(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::SessionView)
    , damageTimePlot_(new DamageTimePlot)
    , xyPositionPlot_(new XYPositionPlot)
    , sessionDataView_(new SessionDataView)
{
    ui_->setupUi(this);

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
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(damageTimePlot_);
    ui_->tab_damage_vs_time->setLayout(layout);

    layout = new QVBoxLayout;
    layout->addWidget(xyPositionPlot_);
    ui_->tab_xy_positions->setLayout(layout);
}

// ----------------------------------------------------------------------------
SessionView::~SessionView()
{
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
    damageTimePlot_->setSession(session);
    xyPositionPlot_->setSession(session);
}

// ----------------------------------------------------------------------------
void SessionView::clear()
{
    sessionDataView_->clear();
    damageTimePlot_->clear();
    xyPositionPlot_->clear();
}

}
