#pragma once

#include "video-player/models/VideoDecoder.hpp"
#include "rfcommon/Deque.hpp"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class AVDecoder;
class VideoPlayerListener;

namespace rfcommon {
    class Log;
}

class BufferedSeekableDecoder
        : public QThread
        , public AVDecoderInterface
{
    Q_OBJECT

public:
    struct FrameEntry
    {
        FrameEntry* next;
        FrameEntry* prev;
        AVFrame* frame;
    };

    explicit BufferedSeekableDecoder(AVDecoder* decoder, QObject* parent);
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

    bool seekToKeyframeBefore(int64_t ts) override;
    bool seekToKeyframeBefore(int64_t ts, int num, int den) override;

    int shortSeek(int deltaFrames);

private:
    void run() override;

private:
    enum State
    {
        IDLE,
        DECODE_FORWARDS,
        QUEUES_FLUSHED
    } state_ = IDLE;

    AVDecoder* decoder_;
    QMutex mutex_;
    QWaitCondition cond_;

    rfcommon::Deque<FrameEntry> vFront_;
    rfcommon::Deque<FrameEntry> vBack_;
    FrameEntry* vCurrent_ = nullptr;

    rfcommon::Deque<FrameEntry> aFront_;
    rfcommon::Deque<FrameEntry> aBack_;
    FrameEntry* aCurrent_ = nullptr;

    rfcommon::FlatFreeList<FrameEntry, 64> freeFrameEntries_;

    bool requestShutdown_ = false;
};
