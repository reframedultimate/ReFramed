#include "video-player/models/AVDecoder.hpp"
#include "video-player/models/BufferedSeekableDecoder.hpp"

#include "rfcommon/Log.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Vector.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

static void printDeque(const char* queueName, rfcommon::Deque<BufferedSeekableDecoder::FrameEntry>* d)
{
    int i = 0;
    printf("%s\n", queueName);
    for (auto it : *d)
        printf("  %d: pts=%ld\n", i++, it->frame->pts);
}

// ----------------------------------------------------------------------------
BufferedSeekableDecoder::BufferedSeekableDecoder(
        AVDecoder* decoder,
        QObject* parent)
    : QThread(parent)
    , decoder_(decoder)
{
}

// ----------------------------------------------------------------------------
BufferedSeekableDecoder::~BufferedSeekableDecoder()
{
}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::openFile(const void* address, uint64_t size)
{
    if (decoder_->openFile(address, size) == false)
        return false;

    requestShutdown_ = false;
    requestSeek_ = false;
    seekRequestFailed_ = false;
    decodeFailed_ = false;

    start();
    return true;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::closeFile()
{
    mutex_.lock();
        requestShutdown_ = true;
        cond_.wakeOne();
    mutex_.unlock();
    wait();

    // Clear queues
    while (vFront_.count())
    {
        FrameEntry* e = vFront_.takeFront();
        decoder_->giveNextVideoFrame(e->frame);
        freeFrameEntries_.put(e);
    }
    while (vBack_.count())
    {
        FrameEntry* e = vBack_.takeFront();
        decoder_->giveNextVideoFrame(e->frame);
        freeFrameEntries_.put(e);
    }

    decoder_->closeFile();
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takeNextVideoFrame()
{
    mutex_.lock();
    cond_.wakeOne();

    decodeFailed_ = false;
    puts("(main) set false and wait 1");
    while (vFront_.count() == 0)
    {
        if (decodeFailed_)
        {
            puts("(main) decode failed 1");
            mutex_.unlock();
            return nullptr;
        }

        cond_.wakeOne();
        mutex_.unlock();
        mutex_.lock();
    }
    puts("(main) decode succeeded 1");

    FrameEntry* e = vFront_.takeBack();
    freeFrameEntries_.put(e);

    mutex_.unlock();

    return e->frame;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::giveNextVideoFrame(AVFrame* frame)
{
    mutex_.lock();
    cond_.wakeOne();

    FrameEntry* e;
    while ((e = freeFrameEntries_.take()) == nullptr)
    {
        cond_.wakeOne();
        mutex_.unlock();
        mutex_.lock();
    }

    e->frame = frame;
    vBack_.putFront(e);

    mutex_.unlock();
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takePrevVideoFrame()
{
    mutex_.lock();
    cond_.wakeOne();

    decodeFailed_ = false;
    puts("(main) set false and wait 2");
    while (vBack_.count() == 0)
    {
        if (decodeFailed_)
        {
            puts("(main) decode failed 2");
            mutex_.unlock();
            return nullptr;
        }

        cond_.wakeOne();
        mutex_.unlock();
        mutex_.lock();
    }
    puts("(main) decode succeeded 2");

    FrameEntry* e = vBack_.takeFront();
    freeFrameEntries_.put(e);

    mutex_.unlock();

    return e->frame;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::givePrevVideoFrame(AVFrame* frame)
{
    mutex_.lock();
    cond_.wakeOne();

    FrameEntry* e;
    while ((e = freeFrameEntries_.take()) == nullptr)
    {
        cond_.wakeOne();
        mutex_.unlock();
        mutex_.lock();
    }

    e->frame = frame;
    vFront_.putBack(e);

    mutex_.unlock();
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takeNextAudioFrame()
{
    return nullptr;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::giveNextAudioFrame(AVFrame* frame)
{
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takePrevAudioFrame()
{
    return nullptr;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::givePrevAudioFrame(AVFrame* frame)
{

}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::seekNearKeyframe(int64_t ts)
{
    mutex_.lock();
    cond_.wakeOne();

    seekPTS_ = ts;
    requestSeek_ = true;

    while (requestSeek_ && !seekRequestFailed_)
    {
        cond_.wakeOne();
        mutex_.unlock();
        mutex_.lock();
    }
    bool success = !seekRequestFailed_;

    mutex_.unlock();

    return success;
}

// ----------------------------------------------------------------------------
int64_t BufferedSeekableDecoder::toCodecTimeStamp(int64_t ts, int num, int den) const
{
    return decoder_->toCodecTimeStamp(ts, num, den);
}

// ----------------------------------------------------------------------------
int64_t BufferedSeekableDecoder::fromCodecTimeStamp(int64_t codec_ts, int num, int den) const
{
    return decoder_->fromCodecTimeStamp(codec_ts, num, den);
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::frameRate(int* num, int* den) const
{
    decoder_->frameRate(num, den);
}

// ----------------------------------------------------------------------------
int BufferedSeekableDecoder::step(int deltaFrames)
{
    return 0;
}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::handleSeekRequest(const int balancedVBackSize)
{
    if (requestSeek_)
    {
        // Clear queues
        while (vFront_.count())
        {
            FrameEntry* e = vFront_.takeFront();
            decoder_->giveNextVideoFrame(e->frame);
            freeFrameEntries_.put(e);
        }
        while (vBack_.count())
        {
            FrameEntry* e = vBack_.takeFront();
            decoder_->giveNextVideoFrame(e->frame);
            freeFrameEntries_.put(e);
        }

        // The target PTS is written to the member variable seekPTS_
        if (decoder_->seekNearKeyframe(seekPTS_) == false)
        {
            seekRequestFailed_ = true;
            return true;  // Can't do any more work
        }

        // We Try to decode the entire back queue in one go
        auto decodedEntries = rfcommon::Vector<FrameEntry*>::makeReserved(balancedVBackSize);
        for (int i = 0; i != balancedVBackSize; ++i)
        {
            FrameEntry* e = freeFrameEntries_.take();
            if (e == nullptr)
                break;
            decodedEntries.push(e);
        }

        mutex_.unlock();
            int numDecoded;
            for (numDecoded = 0; numDecoded != decodedEntries.count(); ++numDecoded)
            {
                AVFrame* frame = decoder_->takeNextVideoFrame();
                if (frame == nullptr)
                    break;
                decodedEntries[numDecoded]->frame = frame;
            }
        mutex_.lock();

        for (int i = 0; i != numDecoded; ++i)
            vBack_.putFront(decodedEntries[i]);
        for (int i = numDecoded; i < decodedEntries.count(); ++i)
            freeFrameEntries_.put(decodedEntries[i]);

        // If back queue is empty (nothing was decoded), abort
        if (vBack_.count() == 0)
        {
            seekRequestFailed_ = true;
            return true;  // Can't do any more work
        }

        // There are a few cases that can occur now. It's possible that
        // the seek landed us far enough before the requested seek PTS
        // where filling the back queue is not enough to reach the target
        // PTS. If this happens then we have to drop the frames from the
        // back and continue decoding forwards until we reach the target
        // PTS.
        while (vBack_.peekFront()->frame->pts < seekPTS_)
        {
            mutex_.unlock();
                AVFrame* frame = decoder_->takeNextVideoFrame();
            mutex_.lock();

            if (frame == nullptr)
            {
                seekRequestFailed_ = true;
                return true;  // Can't do any more work
            }

            FrameEntry* e = vBack_.takeBack();
            decoder_->giveNextVideoFrame(e->frame);
            e->frame = frame;
            vBack_.putFront(e);
        }

        // It's also possible that we've decoded too many frames, in which
        // case we move as many as necessary into the front queue
        while (vBack_.count() && vBack_.peekFront()->frame->pts >= seekPTS_)
            vFront_.putBack(vBack_.takeFront());

        requestSeek_ = false;
        seekRequestFailed_ = false;
    }

    return true;  // Seek successful, no more work
}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::handleDecodeForwards(const int vFrontThreshold, const int balancedVFrontSize)
{
    if (vFront_.count() < balancedVFrontSize)
    {
        puts("(thread) vFront_.count() < vFrontThreshold");
        int64_t decoderTargetPTS;
        if (vFront_.count())
            decoderTargetPTS = vFront_.peekFront()->frame->pts;
        else if (vBack_.count())
            decoderTargetPTS = vBack_.peekFront()->frame->pts;
        else
            decoderTargetPTS = 0;

        AVFrame* frame;
        mutex_.unlock();
            while (1)
            {
                frame = decoder_->takeNextVideoFrame();
                if (frame == nullptr)
                {
                    mutex_.lock();
                    decodeFailed_ = true;
                    puts("(thread) set true 1");
                    return true;  // Can't do any more work
                }

                if (frame->pts > decoderTargetPTS)
                    break;

                decoder_->giveNextVideoFrame(frame);
            }
        mutex_.lock();

        while (1)
        {
            FrameEntry* e = freeFrameEntries_.take();
            if (e == nullptr)
            {
                decoder_->giveNextVideoFrame(frame);
                break;
            }
            e->frame = frame;
            vFront_.putFront(e);

            if (vFront_.count() >= balancedVFrontSize)
                break;

            mutex_.unlock();
                frame = decoder_->takeNextVideoFrame();
                if (frame == nullptr)
                {
                    mutex_.lock();
                    decodeFailed_ = true;
                    puts("(thread) set true 2");
                    return true;  // Can't do any more work
                }
            mutex_.lock();
        }
    }

    return true;  // We're done
}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::handleDecodeBackwards(const int vBackThreshold, const int balancedVBackSize)
{
    if (vBack_.count() < vBackThreshold)
    {
        puts("(thread) vBack_.count() < vBackThreshold");

        int64_t earliestDecodedPTS;
        if (vBack_.count())
            earliestDecodedPTS = vBack_.peekBack()->frame->pts;
        else if (vFront_.count())
            earliestDecodedPTS = vFront_.peekBack()->frame->pts;
        else
        {
            decodeFailed_ = true;
            puts("(thread) set true 3");
            return true;  // Can't do any more work
        }

        // We Try to decode the entire back queue in one go
        rfcommon::Deque<FrameEntry> tmpDeque;
        rfcommon::FreeList<FrameEntry> tmpPool;
        for (int i = 0; i != balancedVBackSize - vBack_.count(); ++i)
        {
            FrameEntry* e = freeFrameEntries_.take();
            if (e == nullptr)
                break;
            tmpPool.put(e);
        }

        mutex_.unlock();
            // Calculate where we need to seek to by subtracting the number
            // of free frames we need to fill in from the current PTS
            int fpsNum, fpsDen;
            decoder_->frameRate(&fpsNum, &fpsDen);
            int seekPTS = earliestDecodedPTS - decoder_->toCodecTimeStamp(tmpPool.count(), fpsDen, fpsNum);
            if (seekPTS < 0)
                seekPTS = 0;

            // Seek to that location
            if (decoder_->seekNearKeyframe(seekPTS) == false)
            {
                mutex_.lock();

                while (tmpPool.peek())
                    freeFrameEntries_.put(tmpPool.take());

                decodeFailed_ = true;
                puts("(thread) set true 4");
                return true;  // Can't do any more work
            }

            AVFrame* frame;
            while (1)
            {
                frame = decoder_->takeNextVideoFrame();
                if (frame == nullptr)
                {
                    mutex_.lock();

                    while (tmpDeque.count())
                        vBack_.putBack(tmpDeque.takeFront());
                    while (tmpPool.peek())
                        freeFrameEntries_.put(tmpPool.take());

                    decodeFailed_ = true;
                    puts("(thread) set true 5");
                    return true;  // Can't do any more work
                }

                if (frame->pts >= earliestDecodedPTS)
                {
                    decoder_->giveNextVideoFrame(frame);
                    break;
                }

                FrameEntry* e = tmpPool.take();
                if (e)
                {
                    e->frame = frame;
                    tmpDeque.putFront(e);
                }
                else
                {
                    e = tmpDeque.takeBack();
                    decoder_->giveNextVideoFrame(e->frame);
                    e->frame = frame;
                    tmpDeque.putFront(e);
                }
            }
        mutex_.lock();

        if (tmpDeque.count() == 0)
        {
            decodeFailed_ = true;
            puts("(thread) set true 6");
        }

        while (tmpDeque.count())
            vBack_.putBack(tmpDeque.takeFront());
        while (tmpPool.peek())
            freeFrameEntries_.put(tmpPool.take());

        if (decodeFailed_)
            return true;  // Can't do any more work
    }

    return true;  // We're done
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::run()
{
    mutex_.lock();
    while (requestShutdown_ == false)
    {
        const int balancedVFrontSize = freeFrameEntries_.capacity() / 2 - 1;
        const int balancedVBackSize = freeFrameEntries_.capacity() / 2;
        const int vFrontThreshold = balancedVFrontSize / 2;
        const int vBackThreshold = balancedVBackSize / 2;

        // Try to balance queues
        while (vFront_.count() > balancedVFrontSize)
        {
            FrameEntry* e = vFront_.takeFront();
            decoder_->giveNextVideoFrame(e->frame);
            freeFrameEntries_.put(e);
        }
        while (vBack_.count() > balancedVBackSize)
        {
            FrameEntry* e = vBack_.takeBack();
            decoder_->giveNextVideoFrame(e->frame);
            freeFrameEntries_.put(e);
        }

        if (handleSeekRequest(balancedVBackSize) == false)
            continue;
        if (handleDecodeForwards(vFrontThreshold, balancedVFrontSize) == false)
            continue;
        if (handleDecodeBackwards(vBackThreshold, balancedVBackSize) == false)
            continue;

        if (requestShutdown_ == false && requestSeek_ == false)
        {
            puts("(thread) idle");
            cond_.wait(&mutex_);
            puts("(thread) wakeup");
        }
    }

    mutex_.unlock();
}
