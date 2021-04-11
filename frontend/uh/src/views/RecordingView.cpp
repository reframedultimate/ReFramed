#include "uh/ui_RecordingView.h"
#include "uh/views/RecordingView.hpp"
#include "uh/views/DamagePlot.hpp"
#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
RecordingView::RecordingView(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingView)
    , plot_(new DamagePlot)
{
    ui_->setupUi(this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(plot_);
    ui_->tab_damage->setLayout(layout);
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

    int count = recording_->playerCount();
    plot_->resetPlot(count);
    for (int i = 0; i != count; ++i)
        plot_->setPlayerName(i, recording_->playerName(i));

    recording_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingPlayerNameChanged(int player, const QString& name)
{
    plot_->setPlayerName(player, name);
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
void RecordingView::onActiveRecordingPlayerStateAdded(int player, const PlayerState& state)
{
    plot_->addPlayerDamageValue(player, state.frame(), state.damage());
    plot_->replotAndAutoScale();
}

}