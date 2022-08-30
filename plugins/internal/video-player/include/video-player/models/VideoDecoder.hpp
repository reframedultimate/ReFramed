#pragma once

#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/FreeList.hpp"
#include "rfcommon/Queue.hpp"

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

extern "C" {
typedef struct AVIOContext AVIOContext;
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodec AVCodec;
typedef struct AVCodecContext AVCodecContext;
typedef struct AVCodecParameters AVCodecParameters;
typedef struct AVStream AVStream;
typedef struct AVPacket AVPacket;
typedef struct AVFrame AVFrame;
typedef struct SwsContext SwsContext;
}

namespace rfcommon {
    class Log;
}

struct FrameEntry
{
    FrameEntry* next;
    AVFrame* frame;
};

class AVDecoder
{
public:
    AVDecoder(rfcommon::Log* log);
    ~AVDecoder();

    bool openFile(const void* address, uint64_t size);
    void closeFile();

    bool isOpen() const { return isOpen_; }

    /*!
     * \brief Decodes the next frame of video and converts it into RGB24.
     */
    AVFrame* takeNextVideoFrame();

    void giveVideoFrame(AVFrame* frame);

    /*!
     * \brief Decodes the next chunk of audio.
     */
    AVFrame* takeNextAudioFrame();

    void giveAudioFrame(AVFrame* frame);

    bool seekToTimeStamp(int64_t ts);
    bool seekToTimeStamp(int64_t ts, int num, int den);

private:
    bool decodeNextPacket();

private:
    rfcommon::Log* log_;

    // libav state
    int sourceWidth_ = 0;
    int sourceHeight_ = 0;
    int videoStreamIdx_ = -1;
    int audioStreamIdx_ = -1;
    AVIOContext* ioCtx_ = nullptr;
    AVFormatContext* inputCtx_ = nullptr;
    AVCodecContext* videoCodecCtx_ = nullptr;
    SwsContext* videoScaleCtx_ = nullptr;
    AVPacket* currentPacket_ = nullptr;

    // queues
    rfcommon::FlatFreeList<FrameEntry, 64> freeFrameEntries_;
    rfcommon::FreeList<FrameEntry> framePool_;
    rfcommon::FreeList<FrameEntry> picturePool_;
    rfcommon::Queue<FrameEntry> audioQueue_;
    rfcommon::Queue<FrameEntry> videoQueue_;

    bool isOpen_ = false;
};
