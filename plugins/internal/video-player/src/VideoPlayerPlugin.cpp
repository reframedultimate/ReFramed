#include "video-player/VideoPlayerPlugin.hpp"
#include "video-player/views/VideoPlayerView.hpp"

VideoPlayerPlugin::VideoPlayerPlugin(RFPluginFactory* factory)
    : Plugin(factory)
{}

VideoPlayerPlugin::~VideoPlayerPlugin() {}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* VideoPlayerPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* VideoPlayerPlugin::realtimeInterface() { return nullptr; }
rfcommon::Plugin::ReplayInterface* VideoPlayerPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* VideoPlayerPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* VideoPlayerPlugin::videoPlayerInterface() { return this; }

// ----------------------------------------------------------------------------
QWidget* VideoPlayerPlugin::createView()
{
    return new VideoPlayerView;
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void VideoPlayerPlugin::onGameSessionLoaded(rfcommon::Session* game) {}
void VideoPlayerPlugin::onGameSessionUnloaded(rfcommon::Session* game) {}
void VideoPlayerPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void VideoPlayerPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void VideoPlayerPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void VideoPlayerPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
