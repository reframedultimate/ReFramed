#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include <QObject>
#include <QImage>
#include <QTimer>
#include <memory>

class VideoDecoder;
class VideoPlayerListener;

class VideoPlayerModel : public QObject
{
    Q_OBJECT

public:
    VideoPlayerModel();
    ~VideoPlayerModel();

    bool open(const void* address, uint64_t size);
    void close();
    void play();
    void pause();
    void advanceFrames(int videoFrames);

    QImage currentFrameAsImage();

    rfcommon::ListenerDispatcher<VideoPlayerListener> dispatcher;

private slots:
    void onPresentNextFrame();
    void onInfo(const QString& msg);
    void onError(const QString& msg);

private:
    std::unique_ptr<VideoDecoder> decoder_;
    QTimer timer_;
};
