#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <QObject>

class BufferedSeekableDecoder;
class VideoPlayerListener;

extern "C" {
typedef struct AVFrame AVFrame;
}

namespace rfcommon {
    class Log;
}

class VideoPlayerModel
        : QObject
        , public rfcommon::Plugin::VideoPlayerInterface
{
    Q_OBJECT

public:
    VideoPlayerModel(BufferedSeekableDecoder* decoder, rfcommon::Log* log);
    ~VideoPlayerModel();

    rfcommon::ListenerDispatcher<VideoPlayerListener> dispatcher;

public:
    bool openVideoFromMemory(const void* data, uint64_t size) override final;
    void closeVideo() override final;
    void playVideo() override final;
    void pauseVideo() override final;
    bool isVideoPlaying() const override final;
    void setVideoVolume(int percent) override final;
    void stepVideo(int videoFrames) override final;
    void seekVideoToGameFrame(rfcommon::FrameIndex frameNumber) override final;
    rfcommon::FrameIndex currentVideoGameFrame() override final;

private:
    BufferedSeekableDecoder* decoder_;
    AVFrame* currentFrame_;
    bool isOpen_;
};
