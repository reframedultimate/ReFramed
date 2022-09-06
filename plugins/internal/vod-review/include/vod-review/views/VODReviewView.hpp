#pragma once

#include <QWidget>

class VideoPlayerModel;

namespace Ui {
    class VODReviewView;
}

class VODReviewView : public QWidget
{
    Q_OBJECT

public:
    explicit VODReviewView(VideoPlayerModel* videoPlayer, QWidget* parent=nullptr);
    ~VODReviewView();

private slots:
    void onPlayPauseReleased();
    void onStepForwardsReleased();
    void onStepBackwardsReleased();

private:
    Ui::VODReviewView* ui_;
    VideoPlayerModel* videoPlayer_;
};
