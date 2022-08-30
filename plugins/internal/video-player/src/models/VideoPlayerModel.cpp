#include "video-player/listeners/VideoPlayerListener.hpp"
#include "video-player/models/VideoPlayerModel.hpp"
#include "video-player/models/VideoDecoder.hpp"

#include "rfcommon/Log.hpp"
#include "rfcommon/Profiler.hpp"

namespace {

class Worker : public QObject
{
public:
    QThread thread;
};
}

// ----------------------------------------------------------------------------
VideoPlayerModel::VideoPlayerModel(rfcommon::Log* log, QObject* parent)
    : QThread(parent)
    , log_(log)
{
}

// ----------------------------------------------------------------------------
VideoPlayerModel::~VideoPlayerModel()
{

}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::open(const void* address, uint64_t size)
{
    PROFILE(VideoPlayerModel, open);

    decoder_.reset(new VideoDecoder(address, size, log_->child("Decoder")));
    if (decoder_->isOpen())
    {
        timer_.setInterval(16);  // TODO sync to audio. Hard coded to 60 fps for now
        connect(&timer_, &QTimer::timeout, this, &VideoPlayerModel::onPresentNextFrame);

        decoder_->seekToGameFrame(rfcommon::FrameIndex::fromValue(120));

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

// ----------------------------------------------------------------------------
void VideoPlayerModel::close()
{
    PROFILE(VideoPlayerModel, close);

    timer_.stop();
    disconnect(&timer_, &QTimer::timeout, this, &VideoPlayerModel::onPresentNextFrame);

    decoder_.reset();
}

// ----------------------------------------------------------------------------
bool VideoPlayerModel::isPlaying() const
{
    PROFILE(VideoPlayerModel, isPlaying);

    if (decoder_.get())
        return timer_.isActive();
    return false;
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::seekToGameFrame(rfcommon::FrameIndex frame)
{
    PROFILE(VideoPlayerModel, seekToFrame);

    if (decoder_.get())
    {
        decoder_->seekToTimeStamp(frame, 1, 60);
    }
    dispatcher.dispatch(&VideoPlayerListener::onPresentCurrentFrame);
}

// ----------------------------------------------------------------------------
QImage VideoPlayerModel::currentFrameAsImage()
{
    PROFILE(VideoPlayerModel, currentFrameAsImage);

    if (decoder_.get())
    {
        QImage(rgbFrame_->data[0], sourceWidth_, sourceHeight_, rgbFrame_->linesize[0], QImage::Format_RGB888);
        return decoder_->currentVideoFrameAsImage();
    }
    return QImage();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::play()
{
    PROFILE(VideoPlayerModel, play);

    if (decoder_.get())
        timer_.start();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::pause()
{
    PROFILE(VideoPlayerModel, pause);

    if (decoder_.get())
        timer_.stop();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::togglePlaying()
{
    if (isPlaying())
        pause();
    else
        play();
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::advanceFrames(int numFrames)
{
    if (decoder_.get() == nullptr)
        return;

    while (numFrames > 0)
    {
        decoder_->nextVideoFrame();
        numFrames--;
    }
    while (numFrames < 0)
    {
        decoder_->prevVideoFrame();
        numFrames++;
    }

    dispatcher.dispatch(&VideoPlayerListener::onPresentCurrentFrame);
}

// ----------------------------------------------------------------------------
void VideoPlayerModel::onPresentNextFrame()
{
    PROFILE(VideoPlayerModel, onPresentNextFrame);

    if (decoder_.get() == nullptr)
        return;

    decoder_->nextVideoFrame();
    dispatcher.dispatch(&VideoPlayerListener::onPresentCurrentFrame);
}
