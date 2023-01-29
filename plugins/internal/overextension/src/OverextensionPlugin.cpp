#include "overextension/OverextensionPlugin.hpp"
#include "overextension/models/OverextensionModel.hpp"
#include "overextension/views/OverextensionView.hpp"

#include "rfcommon/Profiler.hpp"
#include <QWidget>

// ----------------------------------------------------------------------------
OverextensionPlugin::OverextensionPlugin(RFPluginFactory* factory)
    : Plugin(factory)
    , model_(new OverextensionModel)
{
}

// ----------------------------------------------------------------------------
OverextensionPlugin::~OverextensionPlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* OverextensionPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* OverextensionPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* OverextensionPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* OverextensionPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* OverextensionPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* OverextensionPlugin::createView()
{
    PROFILE(OverextensionPlugin, createView);

    return new OverextensionView(model_.get());
}

// ----------------------------------------------------------------------------
void OverextensionPlugin::destroyView(QWidget* view)
{
    PROFILE(OverextensionPlugin, destroyView);

    delete view;
}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void OverextensionPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void OverextensionPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void OverextensionPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onProtocolTrainingStarted(rfcommon::Session* training) {}
void OverextensionPlugin::onProtocolTrainingResumed(rfcommon::Session* training) {}
void OverextensionPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void OverextensionPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void OverextensionPlugin::onProtocolGameStarted(rfcommon::Session* match) {}
void OverextensionPlugin::onProtocolGameResumed(rfcommon::Session* match) {}
void OverextensionPlugin::onProtocolGameEnded(rfcommon::Session* match) {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onGameSessionLoaded(rfcommon::Session* session) {}
void OverextensionPlugin::onGameSessionUnloaded(rfcommon::Session* session) {}
void OverextensionPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void OverextensionPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void OverextensionPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void OverextensionPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
