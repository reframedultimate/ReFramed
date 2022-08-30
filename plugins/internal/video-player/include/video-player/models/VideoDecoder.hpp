#pragma once

#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/FreeList.hpp"
#include "rfcommon/Queue.hpp"

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

class AVDecoderInterface
{
public:
    virtual bool openFile(const void* address, uint64_t size) = 0;
    virtual void closeFile() = 0;

    /*!
     * \brief Decodes the next frame of video and converts it into RGB24.
     */
    virtual AVFrame* takeNextVideoFrame() = 0;

    virtual void giveNextVideoFrame(AVFrame* frame) = 0;

    /*!
     * \brief Decodes the next chunk of audio.
     */
    virtual AVFrame* takeNextAudioFrame() = 0;

    virtual void giveNextAudioFrame(AVFrame* frame) = 0;

    virtual bool seekToKeyframeBefore(int64_t ts) = 0;
    virtual bool seekToKeyframeBefore(int64_t ts, int num, int den) = 0;
};

class AVDecoder : public AVDecoderInterface
{
public:
    struct FrameEntry
    {
        FrameEntry* next;
        AVFrame* frame;
    };

    AVDecoder(rfcommon::Log* log);
    ~AVDecoder();

    bool openFile(const void* address, uint64_t size) override;
    void closeFile() override;

    /*!
     * \brief Decodes the next frame of video and converts it into RGB24.
     */
    AVFrame* takeNextVideoFrame() override;

    void giveNextVideoFrame(AVFrame* frame) override;

    /*!
     * \brief Decodes the next chunk of audio.
     */
    AVFrame* takeNextAudioFrame() override;

    void giveNextAudioFrame(AVFrame* frame) override;

    bool seekToKeyframeBefore(int64_t ts) override;
    bool seekToKeyframeBefore(int64_t ts, int num, int den) override;

    int64_t videoFrameToTimeStamp(int VideoFrameIdx);

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
};
