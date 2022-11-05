#include "vod-review/VODReviewPlugin.hpp"
#include "vod-review/models/AVDecoder.hpp"
#include "vod-review/models/BufferedSeekableDecoder.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"
#include "vod-review/views/VODReviewView.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/VisualizerData.hpp"

// ----------------------------------------------------------------------------
VODReviewPlugin::VODReviewPlugin(RFPluginFactory* factory, rfcommon::VisualizerContext* visCtx, rfcommon::Log* log)
    : Plugin(factory)
    , Plugin::VisualizerInterface(visCtx, factory)
    , log_(log)
    , decoder_(new AVDecoder(log))
    , seekableDecoder_(new BufferedSeekableDecoder(decoder_.get()))
    , videoPlayer_(new VideoPlayerModel(seekableDecoder_.get(), log))
{
    videoPlayer_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
VODReviewPlugin::~VODReviewPlugin()
{
    videoPlayer_->dispatcher.removeListener(this);
}

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

// ----------------------------------------------------------------------------
void VODReviewPlugin::onVisualizerDataChanged()
{
    for (int i = 0; i != visualizerSourceCount(); ++i)
    {
        const auto& timeIntervals = visualizerData(i).timeIntervals;
        if (timeIntervals.count() > 0)
            videoPlayer_->seekVideoToGameFrame(timeIntervals[0].start);
    }
}

// ----------------------------------------------------------------------------
void VODReviewPlugin::onFileOpened() {}
void VODReviewPlugin::onFileClosed() {}
void VODReviewPlugin::onPresentImage(const QImage& image)
{
}
