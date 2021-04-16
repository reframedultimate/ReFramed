#include "uh/ui_RecordingView.h"
#include "uh/views/RecordingView.hpp"
#include "uh/views/RecordingDataView.hpp"
#include "uh/views/DamageTimePlot.hpp"
#include "uh/views/XYPositionPlot.hpp"
#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"

#include <QTreeWidgetItem>

namespace uh {

// ----------------------------------------------------------------------------
RecordingView::RecordingView(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingView)
    , damageTimePlot_(new DamageTimePlot)
    , xyPositionPlot_(new XYPositionPlot)
    , recordingDataView_(new RecordingDataView)
{
    ui_->setupUi(this);

    QVBoxLayout* dataLayout = new QVBoxLayout;
    dataLayout->addWidget(recordingDataView_);
    ui_->tab_data->setLayout(dataLayout);

    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    QMetaObject::invokeMethod(this, "addPlotsToUI", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
void RecordingView::addPlotsToUI()
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(damageTimePlot_);
    ui_->tab_damage_vs_time->setLayout(layout);

    layout = new QVBoxLayout;
    layout->addWidget(xyPositionPlot_);
    ui_->tab_xy_positions->setLayout(layout);
}

// ----------------------------------------------------------------------------
RecordingView::~RecordingView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingView::setRecording(Recording* recording)
{
    recordingDataView_->setRecording(recording);
    damageTimePlot_->setRecording(recording);
    xyPositionPlot_->setRecording(recording);
}

}
