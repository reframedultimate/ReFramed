#include "video-player/VODReviewPlugin.hpp"
#include "video-player/models/AVDecoder.hpp"
#include "video-player/models/BufferedSeekableDecoder.hpp"
#include "video-player/models/VideoPlayerModel.hpp"
#include "video-player/views/VODReviewView.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"

// ----------------------------------------------------------------------------
VODReviewPlugin::VODReviewPlugin(RFPluginFactory* factory, rfcommon::Log* log)
    : Plugin(factory)
    , log_(log)
    , decoder_(new AVDecoder(log))
    , seekableDecoder_(new BufferedSeekableDecoder(decoder_.get()))
    , videoPlayer_(new VideoPlayerModel(seekableDecoder_.get(), log))
{}

// ----------------------------------------------------------------------------
VODReviewPlugin::~VODReviewPlugin()
{}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* VODReviewPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* VODReviewPlugin::realtimeInterface() { return nullptr; }
rfcommon::Plugin::ReplayInterface* VODReviewPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* VODReviewPlugin::visualizerInterface() { return this; }
rfcommon::Plugin::VideoPlayerInterface* VODReviewPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* VODReviewPlugin::createView()
{
    PROFILE(VODReviewPlugin, createView);

    return new VODReviewView(videoPlayer_.get());
}

// ----------------------------------------------------------------------------
void VODReviewPlugin::destroyView(QWidget* view)
{
    PROFILE(VODReviewPlugin, destroyView);

    delete view;
}

// ----------------------------------------------------------------------------
void VODReviewPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    PROFILE(VODReviewPlugin, onGameSessionLoaded);

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
void VODReviewPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    PROFILE(VODReviewPlugin, onGameSessionUnloaded);

    videoPlayer_->closeVideo();
}

void VODReviewPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void VODReviewPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void VODReviewPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void VODReviewPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
