#pragma once

#include "video-player/listeners/VideoPlayerListener.hpp"
#include "rfcommon/String.hpp"
#include <QWidget>
#include <QTimer>

class QPlainTextEdit;
class VideoDecoder;
class VideoSurface;
class VideoPlayerModel;

class VideoPlayerView
    : public QWidget
    , public VideoPlayerListener
{
    Q_OBJECT

public:
    explicit VideoPlayerView(VideoPlayerModel* model, QWidget* parent=nullptr);
    ~VideoPlayerView();

private:
    void onPresentCurrentFrame() override final;
    void onInfo(const QString& msg) override final;
    void onError(const QString& msg) override final;

private:
    VideoPlayerModel* model_;
    QPlainTextEdit* logWidget_;
    VideoSurface* videoSurface_;
};
