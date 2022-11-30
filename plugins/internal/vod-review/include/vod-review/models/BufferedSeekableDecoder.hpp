#pragma once

#include "vod-review/interfaces/AVDecoderInterface.hpp"
#include "rfcommon/Deque.hpp"
#include "rfcommon/FreeList.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class AVDecoder;

namespace rfcommon {
    class Log;
}

class BufferedSeekableDecoder
        : public QThread
        , public AVDecoderInterface
{
public:
    struct FrameEntry
    {
        FrameEntry* next;
        FrameEntry* prev;
        AVFrame* frame;
    };

    explicit BufferedSeekableDecoder(AVDecoder* decoder, QObject* parent=nullptr);
    ~BufferedSeekableDecoder();

    bool openFile(const void* address, uint64_t size) override;
    void closeFile() override;

    AVFrame* takeNextVideoFrame() override;
    void giveNextVideoFrame(AVFrame* frame) override;

    AVFrame* takePrevVideoFrame();
    void givePrevVideoFrame(AVFrame* frame);

    AVFrame* takeNextAudioFrame() override;
    void giveNextAudioFrame(AVFrame* frame) override;

    AVFrame* takePrevAudioFrame();
    void givePrevAudioFrame(AVFrame* frame);

    bool seekNearKeyframe(int64_t ts) override;
    int64_t toCodecTimeStamp(int64_t ts, int num, int den) const override;
    int64_t fromCodecTimeStamp(int64_t codec_ts, int num, int den) const override;
    void frameRate(int* num, int* den) const override;
    int64_t duration() const override;

    int step(int deltaFrames);

private:
    bool handleSeekRequest(const int balancedVBackSize);
    bool handleDecodeForwards(const int vFrontThreshold, const int balancedVFrontSize);
    bool handleDecodeBackwards(const int vBackThreshold, const int balancedVBackSize);
    void run() override;

private:
    AVDecoder* decoder_;
    QMutex mutex_;
    QWaitCondition cond_;

    rfcommon::FlatFreeList<FrameEntry, 128> freeFrameEntries_;

    rfcommon::Deque<FrameEntry> vFront_;
    rfcommon::Deque<FrameEntry> vBack_;

    rfcommon::Deque<FrameEntry> aFront_;
    rfcommon::Deque<FrameEntry> aBack_;

    int64_t seekPTS_;

    bool requestShutdown_ = false;
    bool requestSeek_ = false;
    bool seekRequestFailed_ = false;
    bool decodeFailed_ = false;
};
