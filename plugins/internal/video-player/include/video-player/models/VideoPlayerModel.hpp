#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include <memory>

class VideoDecoder;

class VideoPlayerModel
{
public:
    VideoPlayerModel();
    ~VideoPlayerModel();

    void open(const void* address, uint64_t size);
    void close();
    void play();
    void pause();
    void advanceFrames(int videoFrames);

private:
    std::unique_ptr<VideoDecoder> decoder_;
};
