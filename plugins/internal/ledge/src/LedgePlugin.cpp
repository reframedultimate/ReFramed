#include "ledge/LedgePlugin.hpp"
#include <QWidget>

// ----------------------------------------------------------------------------
LedgePlugin::LedgePlugin(RFPluginFactory* factory)
    : Plugin(factory)
{
}

// ----------------------------------------------------------------------------
LedgePlugin::~LedgePlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* LedgePlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* LedgePlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* LedgePlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* LedgePlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* LedgePlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* LedgePlugin::createView()
{
    return new QWidget();
}

// ----------------------------------------------------------------------------
void LedgePlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void LedgePlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void LedgePlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void LedgePlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void LedgePlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void LedgePlugin::onProtocolTrainingStarted(rfcommon::Session* training) {}
void LedgePlugin::onProtocolTrainingResumed(rfcommon::Session* training) {}
void LedgePlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void LedgePlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void LedgePlugin::onProtocolGameStarted(rfcommon::Session* match) {}
void LedgePlugin::onProtocolGameResumed(rfcommon::Session* match) {}
void LedgePlugin::onProtocolGameEnded(rfcommon::Session* match) {}

// ----------------------------------------------------------------------------
void LedgePlugin::onGameSessionLoaded(rfcommon::Session* session) {}
void LedgePlugin::onGameSessionUnloaded(rfcommon::Session* session) {}
void LedgePlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void LedgePlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void LedgePlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void LedgePlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
