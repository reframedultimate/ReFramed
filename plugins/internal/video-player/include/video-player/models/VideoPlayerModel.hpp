#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/FrameIndex.hpp"
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

    bool isPlaying() const;
    void seekToFrame(rfcommon::FrameIndex frame);

    QImage currentFrameAsImage();

    rfcommon::ListenerDispatcher<VideoPlayerListener> dispatcher;

public slots:
    void play();
    void pause();
    void togglePlaying();
    void advanceFrames(int numFrames);

private slots:
    void onPresentNextFrame();

private:
    rfcommon::Log* log_;
    std::unique_ptr<VideoDecoder> decoder_;
    QTimer timer_;
};
