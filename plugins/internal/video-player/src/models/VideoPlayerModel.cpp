#include "video-player/listeners/VideoPlayerListener.hpp"
#include "video-player/models/VideoPlayerModel.hpp"
#include "video-player/models/VideoDecoder.hpp"

#include "rfcommon/Log.hpp"
#include "rfcommon/Profiler.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
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

    if (AVFrame* frame = decoder_->takeNextVideoFrame())
    {
        vCurrent_ = freeFrameEntries_.take();
        vCurrent_->frame = frame;
    }
    else
    {
        decoder_->closeFile();
        return false;
    }

    requestShutdown_ = false;
    start();
    return true;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::closeFile()
{
    mutex_.lock();
        requestShutdown_ = true;
    mutex_.unlock();
    wait();

    decoder_->closeFile();
    requestShutdown_ = false;
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takeNextVideoFrame()
{
    return nullptr;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::giveNextVideoFrame(AVFrame* frame)
{
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takePrevVideoFrame()
{
    return nullptr;
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::givePrevVideoFrame(AVFrame* frame)
{
}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takeNextAudioFrame()
{

}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::giveNextAudioFrame(AVFrame* frame)
{

}

// ----------------------------------------------------------------------------
AVFrame* BufferedSeekableDecoder::takePrevAudioFrame()
{

}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::givePrevAudioFrame(AVFrame* frame)
{

}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::seekToKeyframeBefore(int64_t ts)
{
}

// ----------------------------------------------------------------------------
bool BufferedSeekableDecoder::seekToKeyframeBefore(int64_t ts, int num, int den)
{
}

// ----------------------------------------------------------------------------
int BufferedSeekableDecoder::shortSeek(int deltaFrames)
{
}

// ----------------------------------------------------------------------------
void BufferedSeekableDecoder::run()
{
    mutex_.lock();
    while (requestShutdown_ == false)
    {
        int expectedVFrontSize = freeFrameEntries_.capacity() / 2 - 1;
        int expectedVBackSize = freeFrameEntries_.capacity() / 2;

        switch (state_)
        {
            case DECODE_FORWARDS: {
                // Try to balance queues (won't always be the case for example when stepping backwards)
                while (vFront_.count() > expectedVFrontSize)
                {
                    FrameEntry* e = vFront_.takeFront();
                    decoder_->giveNextVideoFrame(e->frame);
                    freeFrameEntries_.put(e);
                }
                while (vBack_.count() > expectedVBackSize)
                {
                    FrameEntry* e = vBack_.takeBack();
                    decoder_->giveNextVideoFrame(e->frame);
                    freeFrameEntries_.put(e);
                }

                mutex_.unlock();
                    AVFrame* vFrame = decoder_->takeNextVideoFrame();
                    if (vFrame == nullptr)
                    {
                        mutex_.lock();
                        break;
                    }

                    FrameEntry* e = freeFrameEntries_.take();
                    e->frame = vFrame;
                mutex_.lock();

                if (vBack_.count() == 0)
                    vBack_.putBack(e);
                else if (vFrame->pts  vBack_.peekFront()->frame->pts)
                }
            } break;

            case QUEUES_FLUSHED: {
                // The queues were flushed which means we need to seek backwards
                // and start decoding forwards to fill them up again
                int64_t ts_steps = decoder_->videoFrameToTimeStamp(1);
                int64_t ts = vCurrent_->frame->pts - ts_steps * (expectedVBackSize - vBack_.count() + 1);
                if (ts <= 0)
                    ts = ts_steps;  // This is the earliest seekable frame from my understanding (0 is invalid)

                mutex_.unlock();
                    if (decoder_->seekToKeyframeBefore(ts) == false)
                    {
                        // Not really much we can do. Have to wait for user to seek again.
                        mutex_.lock();
                        state_ = IDLE;
                        break;
                    }

                too_early:
                    AVFrame* vFrame = decoder_->takeNextVideoFrame();
                    if (vFrame == nullptr)
                    {
                        mutex_.lock();
                        state_ = IDLE;
                        break;
                    }
                    if (vFrame->pts < ts)
                    {
                        decoder_->giveNextVideoFrame(vFrame);
                        goto too_early;
                    }
                mutex_.lock();

                FrameEntry* e = freeFrameEntries_.take();
                e->frame = vFrame;
                vBack_.putBack(e);
                state_ = DECODE_FORWARDS;
            } break;

            case IDLE: break;
        }

        cond_.wait(&mutex_);
    }

    mutex_.unlock();
}
