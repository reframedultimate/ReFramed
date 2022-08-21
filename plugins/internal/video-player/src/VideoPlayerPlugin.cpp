#include "video-player/VideoPlayerPlugin.hpp"
#include "video-player/models/VideoPlayerModel.hpp"
#include "video-player/views/VideoPlayerView.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"

// ----------------------------------------------------------------------------
VideoPlayerPlugin::VideoPlayerPlugin(RFPluginFactory* factory)
    : Plugin(factory)
    , videoPlayer_(new VideoPlayerModel)
{}

// ----------------------------------------------------------------------------
VideoPlayerPlugin::~VideoPlayerPlugin() 
{}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* VideoPlayerPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* VideoPlayerPlugin::realtimeInterface() { return nullptr; }
rfcommon::Plugin::ReplayInterface* VideoPlayerPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* VideoPlayerPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* VideoPlayerPlugin::videoPlayerInterface() { return this; }

// ----------------------------------------------------------------------------
QWidget* VideoPlayerPlugin::createView()
{
    PROFILE(VideoPlayerPlugin, createView);

    return new VideoPlayerView(videoPlayer_.get());
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::destroyView(QWidget* view)
{
    PROFILE(VideoPlayerPlugin, destroyView);

    delete view;
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::onGameSessionLoaded(rfcommon::Session* game) 
{
    PROFILE(VideoPlayerPlugin, onGameSessionLoaded);

    auto vmeta = game->tryGetVideoMeta();
    if (!vmeta)
        return;

    if (vmeta->isEmbedded())
    {
        if (auto embed = game->tryGetVideoEmbed())
            activeVideo_ = embed;
    }
    else
    {
#if defined(_WIN32)
#   define SEPARATOR "\\"
#else
#   define SEPARATOR "/"
#endif
        rfcommon::String absFileName = rfcommon::String(vmeta->filePath()) + SEPARATOR + vmeta->fileName();
        rfcommon::Reference<rfcommon::MappedFile> videoFile = new rfcommon::MappedFile;
        if (videoFile->open(absFileName.cStr()) == false)
            return;

        activeVideo_ = new rfcommon::VideoEmbed(videoFile, videoFile->address(), videoFile->size());
    }

    if (activeVideo_)
        openVideoFromMemory(activeVideo_->address(), activeVideo_->size());
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::onGameSessionUnloaded(rfcommon::Session* game) 
{
    PROFILE(VideoPlayerPlugin, onGameSessionUnloaded);

    closeVideo();
}

void VideoPlayerPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void VideoPlayerPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void VideoPlayerPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void VideoPlayerPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}

// ----------------------------------------------------------------------------
bool VideoPlayerPlugin::openVideoFromMemory(const void* address, uint64_t size) 
{
    PROFILE(VideoPlayerPlugin, openVideoFromMemory);

    return videoPlayer_->open(address, size);
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::closeVideo()
{
    PROFILE(VideoPlayerPlugin, closeVideo);

    videoPlayer_->close();
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::playVideo()
{
    PROFILE(VideoPlayerPlugin, playVideo);

    videoPlayer_->play();
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::pauseVideo()
{
    PROFILE(VideoPlayerPlugin, pauseVideo);

    videoPlayer_->pause();
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::setVideoVolume(int percent)
{
    PROFILE(VideoPlayerPlugin, setVideoVolume);

}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::advanceVideoFrames(int videoFrames)
{
    PROFILE(VideoPlayerPlugin, advanceVideoFrames);

    videoPlayer_->advanceFrames(videoFrames);
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::seekVideoToGameFrame(rfcommon::FrameIndex frameNumber)
{
    PROFILE(VideoPlayerPlugin, seekVideoToGameFrame);

}
