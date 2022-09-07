#include "vod-review/listeners/VideoPlayerListener.hpp"
#include "vod-review/models/BufferedSeekableDecoder.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"

#include "rfcommon/Profiler.hpp"

#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
}

// ----------------------------------------------------------------------------
VideoPlayerModel::VideoPlayerModel(BufferedSeekableDecoder* decoder, rfcommon::Log* log)
    : decoder_(decoder)
    , currentFrame_(nullptr)
    , isOpen_(false)
{
    connect(&timer_, &QTimer::timeout, this, &VideoPlayerModel::onTimerTimeout);
}

// ----------------------------------------------------------------------------
VideoPlayerModel::~VideoPlayerModel()
{
    closeVideo();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::onTimerTimeout()
{
    stepVideo(1);
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::openVideoFromMemory(const void* address, uint64_t size)
{
    closeVideo();

    isOpen_ = decoder_->openFile(address, size);
    if (isOpen_)
    {
        int num, den;
        decoder_->frameRate(&num, &den);
        timer_.setInterval(den * 1000 / num);
        dispatcher.dispatch(&VideoPlayerListener::onFileOpened);

        stepVideo(1);

        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::closeVideo()
{
    if (isOpen_ == false)
        return;

    pauseVideo();

    dispatcher.dispatch(&VideoPlayerListener::onPresentImage, QImage());
    if (currentFrame_)
        decoder_->giveNextVideoFrame(currentFrame_);
    currentFrame_ = nullptr;

    decoder_->closeFile();
    isOpen_ = false;

    dispatcher.dispatch(&VideoPlayerListener::onFileClosed);
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::playVideo()
{
    timer_.start();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::pauseVideo()
{
    timer_.stop();
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::isVideoPlaying() const
{
    return timer_.isActive();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::setVideoVolume(int percent)
{
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::stepVideo(int videoFrames)
{
    if (isOpen_ == false)
        return;

    while (videoFrames > 0)
    {
        if (currentFrame_)
            decoder_->giveNextVideoFrame(currentFrame_);
        currentFrame_ = decoder_->takeNextVideoFrame();
        videoFrames--;
    }

    while (videoFrames < 0)
    {
        if (currentFrame_)
            decoder_->givePrevVideoFrame(currentFrame_);
        currentFrame_ = decoder_->takePrevVideoFrame();
        videoFrames++;
    }

    if (currentFrame_ == nullptr)
        return;

    QImage image(currentFrame_->data[0], currentFrame_->width, currentFrame_->height, currentFrame_->linesize[0], QImage::Format_RGB888);
    dispatcher.dispatch(&VideoPlayerListener::onPresentImage, image);
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::seekVideoToGameFrame(rfcommon::FrameIndex frameNumber)
{
    if (isOpen_ == false)
        return;

    // Check if seeking is even necessary, as it's quite expensive
    if (currentFrame_)
    {
        int currentFrame = decoder_->fromCodecTimeStamp(currentFrame_->pts, 1, 60);
        int diff = frameNumber.index() - currentFrame;
        if (std::abs(diff) < 32)
        {
            stepVideo(diff);
            return;
        }
    }

    if (currentFrame_)
        decoder_->giveNextVideoFrame(currentFrame_);

    int64_t targetPTS = decoder_->toCodecTimeStamp(frameNumber.index(), 1, 60);
    if (decoder_->seekNearKeyframe(targetPTS) == false)
    {
        dispatcher.dispatch(&VideoPlayerListener::onPresentImage, QImage());
        return;
    }

    currentFrame_ = decoder_->takeNextVideoFrame();
    if (currentFrame_ == nullptr)
    {
        dispatcher.dispatch(&VideoPlayerListener::onPresentImage, QImage());
        return;
    }

    QImage image(currentFrame_->data[0], currentFrame_->width, currentFrame_->height, currentFrame_->linesize[0], QImage::Format_RGB888);
    dispatcher.dispatch(&VideoPlayerListener::onPresentImage, image);
}

// ----------------------------------------------------------------------------
rfcommon::FrameIndex VideoPlayerModel::currentVideoGameFrame()
{
    if (isOpen_ == false || currentFrame_ == nullptr)
        return rfcommon::FrameIndex::fromValue(0);

    return rfcommon::FrameIndex::fromValue(
                decoder_->fromCodecTimeStamp(currentFrame_->pts, 1, 60));
}
