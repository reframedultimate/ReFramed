#include "video-player/models/VideoPlayerModel.hpp"
#include "video-player/models/VideoDecoder.hpp"

VideoPlayerModel::VideoPlayerModel()
{}

VideoPlayerModel::~VideoPlayerModel()
{}

void VideoPlayerModel::open(const void* address, uint64_t size)
{
    decoder_->openFile(address, size);
}

void VideoPlayerModel::close()
{
    decoder_->closeFile();
}

void VideoPlayerModel::play()
{

}

void VideoPlayerModel::pause()
{

}

void VideoPlayerModel::advanceFrames(int videoFrames)
{

}
