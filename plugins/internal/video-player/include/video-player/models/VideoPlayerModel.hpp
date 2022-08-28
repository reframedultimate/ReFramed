#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include <QObject>
#include <QImage>
#include <QTimer>
#include <memory>

class VideoDecoder;
class VideoPlayerListener;

namespace rfcommon {
    class Log;
}

class VideoPlayerModel : public QObject
{
    Q_OBJECT

public:
    VideoPlayerModel(rfcommon::Log* log);
    ~VideoPlayerModel();

    bool open(const void* address, uint64_t size);
    void close();
    void play();
    void pause();
    void advanceFrames(int gameFrames);

    QImage currentFrameAsImage();

    rfcommon::ListenerDispatcher<VideoPlayerListener> dispatcher;

private slots:
    void onPresentNextFrame();
    void onInfo(const QString& msg);
    void onError(const QString& msg);

private:
    rfcommon::Log* log_;
    std::unique_ptr<VideoDecoder> decoder_;
    QTimer timer_;
};
