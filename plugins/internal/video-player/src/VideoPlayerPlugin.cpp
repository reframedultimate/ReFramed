#include "video-player/VideoPlayerPlugin.hpp"
#include "video-player/views/VideoPlayerView.hpp"

VideoPlayerPlugin::VideoPlayerPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
{}

VideoPlayerPlugin::~VideoPlayerPlugin() {}

QWidget* VideoPlayerPlugin::createView()
{
    return new VideoPlayerView;
}

void VideoPlayerPlugin::destroyView(QWidget* view)
{
    delete view;
}

void VideoPlayerPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void VideoPlayerPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void VideoPlayerPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void VideoPlayerPlugin::onProtocolDisconnectedFromServer() {}

void VideoPlayerPlugin::onProtocolTrainingStarted(rfcommon::Session* training) {}
void VideoPlayerPlugin::onProtocolTrainingResumed(rfcommon::Session* training) {}
void VideoPlayerPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void VideoPlayerPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void VideoPlayerPlugin::onProtocolGameStarted(rfcommon::Session* game) {}
void VideoPlayerPlugin::onProtocolGameResumed(rfcommon::Session* game) {}
void VideoPlayerPlugin::onProtocolGameEnded(rfcommon::Session* game) {}

void VideoPlayerPlugin::onGameSessionLoaded(rfcommon::Session* game) {}
void VideoPlayerPlugin::onGameSessionUnloaded(rfcommon::Session* game) {}
void VideoPlayerPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void VideoPlayerPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void VideoPlayerPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void VideoPlayerPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
