#include "ui_VODReviewView.h"
#include "vod-review/models/VideoPlayerModel.hpp"
#include "vod-review/models/VODReviewModel.hpp"
#include "vod-review/views/VODReviewView.hpp"
#include "vod-review/views/VideoPlayerView.hpp"
#include "vod-review/widgets/TimelineWidget.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/VisualizerData.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/FrameData.hpp"

#include <QProxyStyle>
#include <QShortcut>
#include <QLabel>

#include <qwt_scale_widget.h>

class SliderJumpDirectStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
    {
        PROFILE(VODReviewViewGlobal, styleHint);

        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
            return (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

// ----------------------------------------------------------------------------
VODReviewView::VODReviewView(VODReviewModel* vodReviewModel, VideoPlayerModel* videoPlayer, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::VODReviewView)
    , vodReviewModel_(vodReviewModel)
    , videoPlayer_(videoPlayer)
{
    ui_->setupUi(this);
    ui_->layout_videoSurface->addWidget(new VideoPlayerView(videoPlayer_));
    ui_->slider_videoPos->setStyle(new SliderJumpDirectStyle(ui_->slider_videoPos->style()));

    ui_->pushButton_playPause->setIcon(QIcon::fromTheme("play"));
    ui_->pushButton_stepBackwards->setIcon(QIcon::fromTheme("chevron-left"));
    ui_->pushButton_stepForwards->setIcon(QIcon::fromTheme("chevron-right"));

    QShortcut* togglePlayShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    QShortcut* nextFrameShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    QShortcut* prevFrameShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);

    /*QwtScaleWidget* scaleWidget = new QwtScaleWidget(QwtScaleDraw::BottomScale);
    ui_->horizontalLayout->addWidget(scaleWidget);*/

    updateUITimer_.setInterval(200);

    VODReviewView::onVODReviewVisualizerDataChanged();

    vodReviewModel_->dispatcher.addListener(this);
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
    vodReviewModel_->dispatcher.removeListener(this);
    vodReviewModel_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayPauseReleased()
{
    PROFILE(VODReviewView, onPlayPauseReleased);

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
    PROFILE(VODReviewView, onStepForwardsReleased);

    updateUITimer_.stop();
    videoPlayer_->pauseVideo();
    videoPlayer_->stepVideo(1);
    onUpdateUI();
}

// ----------------------------------------------------------------------------
void VODReviewView::onStepBackwardsReleased()
{
    PROFILE(VODReviewView, onStepBackwardsReleased);

    updateUITimer_.stop();
    videoPlayer_->pauseVideo();
    videoPlayer_->stepVideo(-1);
    onUpdateUI();
}

// ----------------------------------------------------------------------------
void VODReviewView::onSliderValueChanged(int index)
{
    PROFILE(VODReviewView, onSliderValueChanged);

    auto frame = rfcommon::FrameIndex::fromValue(index);
    videoPlayer_->seekVideoToGameFrame(frame + vodReviewModel_->vmeta()->frameOffset());
}

// ----------------------------------------------------------------------------
void VODReviewView::onUpdateUI()
{
    PROFILE(VODReviewView, onUpdateUI);

    const QSignalBlocker blockVideoPos(ui_->slider_videoPos);
    ui_->slider_videoPos->setValue(videoPlayer_->currentVideoGameFrame().index() - vodReviewModel_->vmeta()->frameOffset().index());
}

// ----------------------------------------------------------------------------
void VODReviewView::onFileOpened()
{
    PROFILE(VODReviewView, onFileOpened);

    auto frameCount = videoPlayer_->videoGameFrameCount();
    if (frameCount.index() == 0)
    {
        if (auto fdata = vodReviewModel_->session()->tryGetFrameData())
            frameCount = rfcommon::FrameIndex::fromValue(fdata->frameCount());
        else
            frameCount = rfcommon::FrameIndex::fromValue(1);
    }

    const QSignalBlocker blockVideoPos(ui_->slider_videoPos);
    ui_->slider_videoPos->setRange(0, frameCount.index() - 1);
    ui_->slider_videoPos->setValue(0);

    for (TimelineWidget* timeline : timelineWidgets_)
        timeline->setExtents(0, frameCount.index());

    if (videoPlayer_->isVideoPlaying())
        updateUITimer_.start();
}

// ----------------------------------------------------------------------------
void VODReviewView::onFileClosed()
{
    PROFILE(VODReviewView, onFileClosed);

    updateUITimer_.stop();
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayerPaused()
{
    PROFILE(VODReviewView, onPlayerPaused);

    updateUITimer_.stop();
    ui_->pushButton_playPause->setIcon(QIcon::fromTheme("play"));
}

// ----------------------------------------------------------------------------
void VODReviewView::onPlayerResumed()
{
    PROFILE(VODReviewView, onPlayerResumed);

    updateUITimer_.start();
    ui_->pushButton_playPause->setIcon(QIcon::fromTheme("pause"));
}

// ----------------------------------------------------------------------------
void VODReviewView::onPresentImage(const QImage& image)
{
    PROFILE(VODReviewView, onPresentImage);

}

// ----------------------------------------------------------------------------
void VODReviewView::onVODReviewVisualizerDataChanged()
{
    // QGridLayout::rowCount() only ever increases, so it does not reflect
    // the true number of rows
    const int actualRowCount = ui_->layout_controls->count() / 2; // There are 2 columns

    QLayoutItem* controlsItem = ui_->layout_controls->itemAtPosition(actualRowCount - 1, 0);
    QLayoutItem* sliderItem = ui_->layout_controls->itemAtPosition(actualRowCount - 1, 1);
    ui_->layout_controls->removeItem(controlsItem);
    ui_->layout_controls->removeItem(sliderItem);

    for (int row = 0; row < actualRowCount - 1; ++row)
    {
        QLayoutItem* item = ui_->layout_controls->itemAtPosition(row, 0);
        ui_->layout_controls->removeItem(item);

        QLayoutItem* subItem;
        while ((subItem = item->layout()->takeAt(0)) != nullptr)
        {
            delete subItem->widget();
            delete subItem;
        }

        delete item->widget();
        delete item;

        item = ui_->layout_controls->itemAtPosition(row, 1);
        ui_->layout_controls->removeItem(item);
        delete item->widget();
        delete item;
    }
    timelineWidgets_.clear();

    for (int i = 0; i != vodReviewModel_->visualizerSourceCount(); ++i)
    {
        TimelineWidget* timeline = new TimelineWidget;
        if (videoPlayer_->isVideoOpen())
        {
            auto frameCount = videoPlayer_->videoGameFrameCount();
            if (frameCount.index() == 0)
            {
                if (auto fdata = vodReviewModel_->session()->tryGetFrameData())
                    frameCount = rfcommon::FrameIndex::fromValue(fdata->frameCount());
                else
                    frameCount = rfcommon::FrameIndex::fromValue(1);
            }
            timeline->setExtents(0, frameCount.index());
        }
        for (const auto& interval : vodReviewModel_->visualizerData(i).timeIntervals)
            timeline->addInterval(interval.start.index(), interval.end.index());
        timelineWidgets_.push_back(timeline);

        QLabel* timelineName = new QLabel(QString::fromUtf8(vodReviewModel_->visualizerName(i)));
        QPushButton* jumpPrev = new QPushButton;
        jumpPrev->setIcon(QIcon::fromTheme("jump-left"));
        QPushButton* jumpNext = new QPushButton;
        jumpPrev->setIcon(QIcon::fromTheme("jump-right"));

        QHBoxLayout* timelineControlsLayout = new QHBoxLayout;
        timelineControlsLayout->addWidget(timelineName);
        timelineControlsLayout->addWidget(jumpPrev);
        timelineControlsLayout->addWidget(jumpNext);

        ui_->layout_controls->addLayout(timelineControlsLayout, i, 0);
        ui_->layout_controls->addWidget(timeline, i, 1);

        connect(jumpPrev, &QPushButton::released, [this, i] {
            if (videoPlayer_->isVideoOpen() == false)
                return;
            
            const auto frame = videoPlayer_->currentVideoGameFrame() - vodReviewModel_->vmeta()->frameOffset();
            const auto& intervals = vodReviewModel_->visualizerData(i).timeIntervals;
            for (int n = 1; n < intervals.count(); ++n)
                if (frame < intervals[n].start)
                {
                    if (videoPlayer_->isVideoPlaying() && n > 1 && frame >= intervals[n-1].start && frame < intervals[n-1].end)
                        videoPlayer_->seekVideoToGameFrame(intervals[n-2].start + vodReviewModel_->vmeta()->frameOffset());
                    else if (videoPlayer_->isVideoPlaying() == false && n > 1 && frame == intervals[n-1].start)
                        videoPlayer_->seekVideoToGameFrame(intervals[n-2].start + vodReviewModel_->vmeta()->frameOffset());
                    else
                        videoPlayer_->seekVideoToGameFrame(intervals[n-1].start + vodReviewModel_->vmeta()->frameOffset());
                    onUpdateUI();
                    return;
                }
            
            if (intervals.count() > 0)
            {
                int n = intervals.count();
                if (videoPlayer_->isVideoPlaying() && n > 1 && frame >= intervals[n-1].start && frame < intervals[n-1].end)
                    videoPlayer_->seekVideoToGameFrame(intervals[n-2].start + vodReviewModel_->vmeta()->frameOffset());
                else if (videoPlayer_->isVideoPlaying() == false && n > 1 && frame == intervals[n-1].start)
                    videoPlayer_->seekVideoToGameFrame(intervals[n-2].start + vodReviewModel_->vmeta()->frameOffset());
                else
                    videoPlayer_->seekVideoToGameFrame(intervals[n-1].start + vodReviewModel_->vmeta()->frameOffset());
                onUpdateUI();
            }
        });

        connect(jumpNext, &QPushButton::released, [this, i] {
            if (videoPlayer_->isVideoOpen() == false)
                return;
            
            const auto frame = videoPlayer_->currentVideoGameFrame() - vodReviewModel_->vmeta()->frameOffset();
            const auto& intervals = vodReviewModel_->visualizerData(i).timeIntervals;
            for (int n = intervals.count() - 2; n >= 0; --n)
                if (frame >= intervals[n].start)
                {
                    videoPlayer_->seekVideoToGameFrame(intervals[n+1].start + vodReviewModel_->vmeta()->frameOffset());
                    onUpdateUI();
                    return;
                }

            if (intervals.count() > 0)
            {
                videoPlayer_->seekVideoToGameFrame(intervals[0].start + vodReviewModel_->vmeta()->frameOffset());
                onUpdateUI();
            }
        });
    }

    ui_->layout_controls->addItem(controlsItem, vodReviewModel_->visualizerSourceCount(), 0);
    ui_->layout_controls->addItem(sliderItem, vodReviewModel_->visualizerSourceCount(), 1);
}
