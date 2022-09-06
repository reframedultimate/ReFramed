#pragma once

#include <cstdint>

extern "C" {
typedef struct AVFrame AVFrame;
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

    virtual bool seekNearKeyframe(int64_t codec_ts) = 0;

    virtual int64_t toCodecTimeStamp(int64_t ts, int num, int den) const = 0;
    virtual int64_t fromCodecTimeStamp(int64_t codec_ts, int num, int den) const = 0;
    virtual void frameRate(int* num, int* den) const = 0;
};
