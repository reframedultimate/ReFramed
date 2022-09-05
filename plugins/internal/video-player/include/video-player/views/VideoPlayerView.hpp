#pragma once

#include "video-player/listeners/VideoPlayerListener.hpp"
#include <QWidget>

class QPlainTextEdit;
class VideoDecoder;
class VideoSurface;
class VideoPlayerModel;

typedef struct AVFrame AVFrame;

class VideoPlayerView
    : public QWidget
    , public VideoPlayerListener
{
    Q_OBJECT

public:
    explicit VideoPlayerView(VideoPlayerModel* model, QWidget* parent=nullptr);
    ~VideoPlayerView();

private:
    void onFileOpened() override final;
    void onFileClosed() override final;
    void onPresentImage(const QImage& image) override final;

private:
    VideoPlayerModel* model_;
    QPlainTextEdit* logWidget_;
    VideoSurface* videoSurface_;
    AVFrame* currentFrame_ = nullptr;
};
