#include "video-player/listeners/VideoPlayerListener.hpp"
#include "video-player/models/BufferedSeekableDecoder.hpp"
#include "video-player/models/VideoPlayerModel.hpp"

#include "rfcommon/Profiler.hpp"

#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
}

// ----------------------------------------------------------------------------
VideoPlayerModel::VideoPlayerModel(BufferedSeekableDecoder* decoder, rfcommon::Log* log)
    : decoder_(decoder)
    , currentFrame_(nullptr)
{
}

// ----------------------------------------------------------------------------
VideoPlayerModel::~VideoPlayerModel()
{
    closeVideo();
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::openVideoFromMemory(const void* address, uint64_t size)
{
    isOpen_ = decoder_->openFile(address, size);
    if (isOpen_)
    {
        dispatcher.dispatch(&VideoPlayerListener::onFileOpened);
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::closeVideo()
{
    if (isOpen_ == false)
        return;

    if (currentFrame_)
        decoder_->giveNextVideoFrame(currentFrame_);
    currentFrame_ = nullptr;

    decoder_->closeFile();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::playVideo()
{
    //videoPlayer_->play();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::pauseVideo()
{
    //videoPlayer_->pause();
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::isVideoPlaying() const
{
    return false;
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

    if (currentFrame_)
        decoder_->giveNextVideoFrame(currentFrame_);

    int64_t targetPTS = decoder_->toCodecTimeStamp(frameNumber.index(), 1, 60);
    if (decoder_->seekNearKeyframe(targetPTS) == false)
        return;

    currentFrame_ = decoder_->takeNextVideoFrame();
    if (currentFrame_ == nullptr)
        return;

    QImage image(currentFrame_->data[0], currentFrame_->width, currentFrame_->height, currentFrame_->linesize[0], QImage::Format_RGB888);
    dispatcher.dispatch(&VideoPlayerListener::onPresentImage, image);
}

// ----------------------------------------------------------------------------
rfcommon::FrameIndex VideoPlayerModel::currentVideoGameFrame()
{
    return rfcommon::FrameIndex::fromValue(0);
}
