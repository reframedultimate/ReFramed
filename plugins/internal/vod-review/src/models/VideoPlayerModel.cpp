#include "vod-review/listeners/VideoPlayerListener.hpp"
#include "vod-review/models/AVDecoder.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"

#include "rfcommon/Profiler.hpp"

#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
}

// ----------------------------------------------------------------------------
VideoPlayerModel::VideoPlayerModel(AVDecoderInterface* decoder, rfcommon::Log* log)
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
    PROFILE(VideoPlayerModel, onTimerTimeout);

    stepVideo(1);
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::openVideoFromMemory(const void* address, uint64_t size)
{
    PROFILE(VideoPlayerModel, openVideoFromMemory);

    closeVideo();

    isOpen_ = decoder_->openFile(address, size);
    if (isOpen_)
    {
        int num, den;
        decoder_->frameRate(&num, &den);
        timer_.setInterval(den * 1000 / num);
        dispatcher.dispatch(&VideoPlayerListener::onFileOpened);

        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::closeVideo()
{
    PROFILE(VideoPlayerModel, closeVideo);

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
bool VideoPlayerModel::isVideoOpen() const
{
    return isOpen_;
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::playVideo()
{
    PROFILE(VideoPlayerModel, playVideo);

    timer_.start();
    dispatcher.dispatch(&VideoPlayerListener::onPlayerResumed);
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::pauseVideo()
{
    PROFILE(VideoPlayerModel, pauseVideo);

    timer_.stop();
    dispatcher.dispatch(&VideoPlayerListener::onPlayerPaused);
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::isVideoPlaying() const
{
    PROFILE(VideoPlayerModel, isVideoPlaying);

    return timer_.isActive();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::setVideoVolume(int percent)
{
    PROFILE(VideoPlayerModel, setVideoVolume);

}

// ----------------------------------------------------------------------------
void VideoPlayerModel::stepVideo(int videoFrames)
{
    PROFILE(VideoPlayerModel, stepVideo);

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
        int64_t targetPTS = 0;
        if (currentFrame_)
        {
            int num, den;
            decoder_->frameRate(&den, &num);  // time base is 1/framerate
            targetPTS = currentFrame_->pts - decoder_->toCodecTimeStamp(-videoFrames, num, den);
            if (targetPTS < 0)
                targetPTS = 0;

            decoder_->givePrevVideoFrame(currentFrame_);
        }

        currentFrame_ = decoder_->takePrevVideoFrame();
        if (currentFrame_ == nullptr)
        {
            if (decoder_->seekNearKeyframe(targetPTS))
            {
                while (1)
                {
                    currentFrame_ = decoder_->takeNextVideoFrame();
                    if (currentFrame_ == nullptr || currentFrame_->pts >= targetPTS)
                        break;
                    decoder_->giveNextVideoFrame(currentFrame_);
                }
            }
            break;
        }

        videoFrames++;
    }

    if (currentFrame_ == nullptr)
        dispatcher.dispatch(&VideoPlayerListener::onPresentImage, QImage());
    else
    {
        QImage image(currentFrame_->data[0], currentFrame_->width, currentFrame_->height, currentFrame_->linesize[0], QImage::Format_RGB888);
        dispatcher.dispatch(&VideoPlayerListener::onPresentImage, image);
    }
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::seekVideoToGameFrame(rfcommon::FrameIndex frameNumber)
{
    PROFILE(VideoPlayerModel, seekVideoToGameFrame);

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
    currentFrame_ = nullptr;

    int64_t targetPTS = decoder_->toCodecTimeStamp(frameNumber.index(), 1, 60);
    if (decoder_->seekNearKeyframe(targetPTS))
    {
        while (1)
        {
            currentFrame_ = decoder_->takeNextVideoFrame();
            if (currentFrame_ == nullptr || currentFrame_->pts >= targetPTS)
                break;
            decoder_->giveNextVideoFrame(currentFrame_);
        }
    }

    if (currentFrame_ == nullptr)
        dispatcher.dispatch(&VideoPlayerListener::onPresentImage, QImage());
    else
    {
        QImage image(currentFrame_->data[0], currentFrame_->width, currentFrame_->height, currentFrame_->linesize[0], QImage::Format_RGB888);
        dispatcher.dispatch(&VideoPlayerListener::onPresentImage, image);
    }
}

// ----------------------------------------------------------------------------
rfcommon::FrameIndex VideoPlayerModel::currentVideoGameFrame() const
{
    PROFILE(VideoPlayerModel, currentVideoGameFrame);

    if (isOpen_ == false || currentFrame_ == nullptr)
        return rfcommon::FrameIndex::fromValue(0);

    return rfcommon::FrameIndex::fromValue(
            decoder_->fromCodecTimeStamp(currentFrame_->pts, 1, 60));
}

// ----------------------------------------------------------------------------
rfcommon::FrameIndex VideoPlayerModel::videoGameFrameCount() const
{
    PROFILE(VideoPlayerModel, videoGameFrameCount);

    return rfcommon::FrameIndex::fromValue(
            decoder_->fromCodecTimeStamp(decoder_->duration(), 1, 60));
}
