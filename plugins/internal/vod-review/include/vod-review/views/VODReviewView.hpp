#pragma once

#include "vod-review/listeners/VideoPlayerListener.hpp"
#include <QWidget>
#include <QTimer>

class VideoPlayerModel;

namespace Ui {
    class VODReviewView;
}

class VODReviewView
        : public QWidget
        , public VideoPlayerListener
{
    Q_OBJECT

public:
    explicit VODReviewView(VideoPlayerModel* videoPlayer, QWidget* parent=nullptr);
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
    Ui::VODReviewView* ui_;
    VideoPlayerModel* videoPlayer_;
    QTimer updateUITimer_;
};
