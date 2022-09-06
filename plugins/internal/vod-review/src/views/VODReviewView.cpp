#include "ui_VODReviewView.h"
#include "vod-review/views/VODReviewView.hpp"
#include "vod-review/views/VideoPlayerView.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"

// ----------------------------------------------------------------------------
VODReviewView::VODReviewView(VideoPlayerModel* videoPlayer, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::VODReviewView)
    , videoPlayer_(videoPlayer)
{
    ui_->setupUi(this);

    ui_->layout_videoSurface->addWidget(new VideoPlayerView(videoPlayer_));

    connect(ui_->pushButton_playPause, &QPushButton::released, this, &VODReviewView::onPlayPauseReleased);
    connect(ui_->pushButton_stepForwards, &QPushButton::released, this, &VODReviewView::onStepForwardsReleased);
    connect(ui_->pushButton_stepBackwards, &QPushButton::released, this, &VODReviewView::onStepBackwardsReleased);
}

// ----------------------------------------------------------------------------
VODReviewView::~VODReviewView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayPauseReleased()
{
    if (videoPlayer_->isVideoPlaying())
        videoPlayer_->pauseVideo();
    else
        videoPlayer_->playVideo();
}

// ----------------------------------------------------------------------------
void VODReviewView::onStepForwardsReleased()
{
    videoPlayer_->pauseVideo();
    videoPlayer_->stepVideo(1);
}

// ----------------------------------------------------------------------------
void VODReviewView::onStepBackwardsReleased()
{
    videoPlayer_->pauseVideo();
    videoPlayer_->stepVideo(-1);
}
