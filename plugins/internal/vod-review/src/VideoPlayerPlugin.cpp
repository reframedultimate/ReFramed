#include "vod-review/VideoPlayerPlugin.hpp"
#include "vod-review/models/AVDecoder.hpp"
#include "vod-review/models/BufferedSeekableDecoder.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"
#include "vod-review/views/VideoPlayerView.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"

// ----------------------------------------------------------------------------
VideoPlayerPlugin::VideoPlayerPlugin(RFPluginFactory* factory, rfcommon::Log* log)
    : Plugin(factory)
    , log_(log)
    , decoder_(new AVDecoder(log))
    , seekableDecoder_(new BufferedSeekableDecoder(decoder_.get()))
    , videoPlayer_(new VideoPlayerModel(decoder_.get(), log))
{}

// ----------------------------------------------------------------------------
VideoPlayerPlugin::~VideoPlayerPlugin()
{}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* VideoPlayerPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* VideoPlayerPlugin::realtimeInterface() { return nullptr; }
rfcommon::Plugin::ReplayInterface* VideoPlayerPlugin::replayInterface() { return this; }
rfcommon::Plugin::SharedDataInterface* VideoPlayerPlugin::sharedInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* VideoPlayerPlugin::videoPlayerInterface() { return videoPlayer_.get(); }

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

    activeVideo_ = game->tryGetVideo();

    if (activeVideo_.isNull())
        return;

    if (videoPlayer_->openVideoFromMemory(activeVideo_->address(), activeVideo_->size()) == false)
    {
        activeVideo_.drop();
        return;
    }

    if (auto vmeta = game->tryGetVideoMeta())
        videoPlayer_->seekVideoToGameFrame(vmeta->frameOffset());

    videoPlayer_->playVideo();
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    PROFILE(VideoPlayerPlugin, onGameSessionUnloaded);

    videoPlayer_->closeVideo();
}

void VideoPlayerPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void VideoPlayerPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void VideoPlayerPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void VideoPlayerPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
