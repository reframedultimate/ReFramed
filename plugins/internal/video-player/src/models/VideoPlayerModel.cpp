#include "rfcommon/Profiler.hpp"
#include "video-player/listeners/VideoPlayerListener.hpp"
#include "video-player/models/VideoPlayerModel.hpp"
#include "video-player/models/VideoDecoder.hpp"

VideoPlayerModel::VideoPlayerModel()
{
    timer_.setInterval(16);
}

VideoPlayerModel::~VideoPlayerModel()
{}

bool VideoPlayerModel::open(const void* address, uint64_t size)
{
    PROFILE(VideoPlayerModel, open);

    decoder_.reset(new VideoDecoder(address, size));
    if (decoder_->isOpen())
    {
        timer_.setInterval(16);  // TODO get fps from decoder
        connect(&timer_, &QTimer::timeout, this, &VideoPlayerModel::onPresentNextFrame);

        //timer_.start();
        dispatcher.dispatch(&VideoPlayerListener::onPresentCurrentFrame);
        return true;
    }
    else
    {
        decoder_.reset();
        return false;
    }
}

void VideoPlayerModel::close()
{
    PROFILE(VideoPlayerModel, close);

    timer_.stop();
    disconnect(&timer_, &QTimer::timeout, this, &VideoPlayerModel::onPresentNextFrame);

    decoder_.reset();
}

void VideoPlayerModel::play()
{
    PROFILE(VideoPlayerModel, play);

    if (decoder_.get())
        timer_.start();
}

void VideoPlayerModel::pause()
{
    PROFILE(VideoPlayerModel, pause);

    if (decoder_.get())
        timer_.stop();
}

void VideoPlayerModel::advanceFrames(int videoFrames)
{
    PROFILE(VideoPlayerModel, advanceFrames);


}

QImage VideoPlayerModel::currentFrameAsImage()
{
    PROFILE(VideoPlayerModel, currentFrameAsImage);

    if (decoder_.get())
        return decoder_->currentFrameAsImage();
    return QImage();
}

void VideoPlayerModel::onPresentNextFrame()
{
    PROFILE(VideoPlayerModel, onPresentNextFrame);

    if (decoder_.get() == nullptr)
        return;

    decoder_->nextFrame();
    dispatcher.dispatch(&VideoPlayerListener::onPresentCurrentFrame);
}

void VideoPlayerModel::onInfo(const QString& msg)
{
    PROFILE(VideoPlayerModel, onInfo);

    dispatcher.dispatch(&VideoPlayerListener::onInfo, msg);
}

void VideoPlayerModel::onError(const QString& msg)
{
    PROFILE(VideoPlayerModel, onError);

    dispatcher.dispatch(&VideoPlayerListener::onError, msg);
}
