#include "ui_VODReviewView.h"
#include "vod-review/views/VODReviewView.hpp"
#include "vod-review/views/VideoPlayerView.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"

#include <QProxyStyle>
#include <QShortcut>

#include <qwt_scale_widget.h>

class SliderJumpDirectStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
    {
        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
            return (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

// ----------------------------------------------------------------------------
VODReviewView::VODReviewView(VideoPlayerModel* videoPlayer, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::VODReviewView)
    , videoPlayer_(videoPlayer)
{
    ui_->setupUi(this);
    ui_->layout_videoSurface->addWidget(new VideoPlayerView(videoPlayer_));
    ui_->slider_videoPos->setStyle(new SliderJumpDirectStyle(ui_->slider_videoPos->style()));

    QShortcut* togglePlayShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    QShortcut* nextFrameShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    QShortcut* prevFrameShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);

    QwtScaleWidget* scaleWidget = new QwtScaleWidget(QwtScaleDraw::BottomScale);
    ui_->horizontalLayout->addWidget(scaleWidget);

    updateUITimer_.setInterval(200);

    videoPlayer_->dispatcher.addListener(this);

    connect(ui_->pushButton_playPause, &QPushButton::released, this, &VODReviewView::onPlayPauseReleased);
    connect(ui_->pushButton_stepForwards, &QPushButton::released, this, &VODReviewView::onStepForwardsReleased);
    connect(ui_->pushButton_stepBackwards, &QPushButton::released, this, &VODReviewView::onStepBackwardsReleased);
    connect(ui_->slider_videoPos, &QSlider::valueChanged, this, &VODReviewView::onSliderValueChanged);
    connect(togglePlayShortcut, &QShortcut::activated, this, &VODReviewView::onPlayPauseReleased);
    connect(nextFrameShortcut, &QShortcut::activated, this, &VODReviewView::onStepForwardsReleased);
    connect(prevFrameShortcut, &QShortcut::activated, this, &VODReviewView::onStepBackwardsReleased);
    connect(&updateUITimer_, &QTimer::timeout, this, &VODReviewView::onUpdateUI);
}

// ----------------------------------------------------------------------------
VODReviewView::~VODReviewView()
{
    videoPlayer_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayPauseReleased()
{
    if (videoPlayer_->isVideoPlaying())
    {
        updateUITimer_.stop();
        videoPlayer_->pauseVideo();
    }
    else
    {
        updateUITimer_.start();
        videoPlayer_->playVideo();
    }
}

// ----------------------------------------------------------------------------
void VODReviewView::onStepForwardsReleased()
{
    updateUITimer_.stop();
    videoPlayer_->pauseVideo();
    videoPlayer_->stepVideo(1);
    onUpdateUI();
}

// ----------------------------------------------------------------------------
void VODReviewView::onStepBackwardsReleased()
{
    updateUITimer_.stop();
    videoPlayer_->pauseVideo();
    videoPlayer_->stepVideo(-1);
    onUpdateUI();
}

// ----------------------------------------------------------------------------
void VODReviewView::onSliderValueChanged(int index)
{
    auto frame = rfcommon::FrameIndex::fromValue(index);
    videoPlayer_->seekVideoToGameFrame(frame);
}

// ----------------------------------------------------------------------------
void VODReviewView::onUpdateUI()
{
    bool store = ui_->slider_videoPos->blockSignals(true);
    ui_->slider_videoPos->setValue(videoPlayer_->currentVideoGameFrame().index());
    ui_->slider_videoPos->blockSignals(store);
}

// ----------------------------------------------------------------------------
void VODReviewView::onFileOpened()
{
    auto frameCount = videoPlayer_->videoGameFrameCount();
    ui_->slider_videoPos->setRange(0, frameCount.index() - 1);
    ui_->slider_videoPos->setValue(videoPlayer_->currentVideoGameFrame().index());

    if (videoPlayer_->isVideoPlaying())
        updateUITimer_.start();
}

// ----------------------------------------------------------------------------
void VODReviewView::onFileClosed()
{
    updateUITimer_.stop();
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayerPaused()
{
    updateUITimer_.stop();
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayerResumed()
{
    updateUITimer_.start();
}

// ----------------------------------------------------------------------------
void VODReviewView::onPresentImage(const QImage& image)
{
}
