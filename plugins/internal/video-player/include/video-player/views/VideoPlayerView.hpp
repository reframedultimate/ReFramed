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

private slots:
    void drawNextFrame();
    void info(const QString& msg);
    void error(const QString& msg);

private:
    QTimer timer_;
    QPlainTextEdit* logWidget_;
    VideoSurface* videoSurface_;
    VideoDecoder* decoder_;
};
