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
    if (recording_)
        recording_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingView::setRecording(Recording* recording)
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    recording_ = recording;

    recordingDataView_->setRecording(recording);

    // Plot existing data
    int playerCount = recording_->playerCount();
    damageTimePlot_->resetPlot(playerCount);
    for (int i = 0; i != playerCount; ++i)
    {
        damageTimePlot_->setPlayerName(i, recording_->playerName(i));
        for (const auto& state : recording_->playerStates(i))
            damageTimePlot_->addPlayerDamageValue(i, state.frame(), state.damage());
    }
    damageTimePlot_->forceAutoScale();

    xyPositionPlot_->setRecording(recording);

    recording_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingPlayerNameChanged(int player, const QString& name)
{
    damageTimePlot_->setPlayerName(player, name);
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingSetNumberChanged(int number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingGameNumberChanged(int number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingFormatChanged(const SetFormat& format)
{
    (void)format;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state)
{
    (void)player;
    (void)state;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingNewPlayerState(int player, const PlayerState& state)
{
    // Update plot
    damageTimePlot_->addPlayerDamageValue(player, state.frame(), state.damage());
    damageTimePlot_->conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void RecordingView::onRecordingWinnerChanged(int winner)
{
    (void)winner;
}

}
