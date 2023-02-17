#include "rfcommon/Profiler.hpp"
#include "streameta/StreametaPlugin.hpp"
#include <QWidget>

// ----------------------------------------------------------------------------
StreametaPlugin::StreametaPlugin(RFPluginFactory* factory)
    : Plugin(factory)
{
}

// ----------------------------------------------------------------------------
StreametaPlugin::~StreametaPlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* StreametaPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* StreametaPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* StreametaPlugin::replayInterface() { return nullptr; }
rfcommon::Plugin::SharedDataInterface* StreametaPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* StreametaPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* StreametaPlugin::createView() 
{
    PROFILE(StreametaPlugin, createView);
 
    return new QWidget();
}

// ----------------------------------------------------------------------------
void StreametaPlugin::destroyView(QWidget* view) 
{
    PROFILE(StreametaPlugin, destroyView);
 
    delete view; 
}

// ----------------------------------------------------------------------------
void StreametaPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void StreametaPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void StreametaPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void StreametaPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void StreametaPlugin::onProtocolTrainingStarted(rfcommon::Session* training) {}
void StreametaPlugin::onProtocolTrainingResumed(rfcommon::Session* training) {}
void StreametaPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void StreametaPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void StreametaPlugin::onProtocolGameStarted(rfcommon::Session* match) {}
void StreametaPlugin::onProtocolGameResumed(rfcommon::Session* match) {}
void StreametaPlugin::onProtocolGameEnded(rfcommon::Session* match) {}
