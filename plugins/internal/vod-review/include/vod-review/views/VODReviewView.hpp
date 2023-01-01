#pragma once

#include "vod-review/listeners/VideoPlayerListener.hpp"
#include "vod-review/listeners/VODReviewListener.hpp"
#include <QWidget>
#include <QTimer>

class VideoPlayerModel;
class VODReviewModel;

namespace Ui {
    class VODReviewView;
}

class TimelineWidget;

class VODReviewView
        : public QWidget
        , public VideoPlayerListener
        , public VODReviewListener
{
    Q_OBJECT

public:
    explicit VODReviewView(VODReviewModel* vodReviewModel, VideoPlayerModel* videoPlayer, QWidget* parent=nullptr);
    ~VODReviewView();

private slots:
    void onPlayPauseReleased();
    void onStepForwardsReleased();
    void onStepBackwardsReleased();
    void onSliderValueChanged(int value);
    void onUpdateUI();

private:
    void onFileOpened() override final;
    void onFileClosed() override final;
    void onPlayerPaused() override final;
    void onPlayerResumed() override final;
    void onPresentImage(const QImage& image) override final;

private:
    void onVODReviewVisualizerDataChanged() override final;

private:
    Ui::VODReviewView* ui_;
    VODReviewModel* vodReviewModel_;
    VideoPlayerModel* videoPlayer_;
    QTimer updateUITimer_;
    QVector<TimelineWidget*> timelineWidgets_;
};
