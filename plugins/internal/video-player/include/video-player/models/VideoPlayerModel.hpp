#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/FrameIndex.hpp"
#include <QThread>
#include <QTimer>
#include <memory>

class AVDecoder;
class VideoPlayerListener;

namespace rfcommon {
    class Log;
}

class VideoPlayerModel : public QThread
{
    Q_OBJECT

public:
    explicit VideoPlayerModel(rfcommon::Log* log, QObject* parent);
    ~VideoPlayerModel();

    bool open(const void* address, uint64_t size);
    void close();

    bool isPlaying() const;
    void seekToGameFrame(rfcommon::FrameIndex frame);

    rfcommon::ListenerDispatcher<VideoPlayerListener> dispatcher;

public slots:
    void play();
    void pause();
    void togglePlaying();
    void advanceFrames(int numFrames);

private:
    void run() override;

private:
    rfcommon::Log* log_;
    std::unique_ptr<AVDecoder> decoder_;
    QTimer timer_;
};
