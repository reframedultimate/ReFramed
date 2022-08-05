#include "streameta/StreametaPlugin.hpp"
#include <QWidget>

// ----------------------------------------------------------------------------
StreametaPlugin::StreametaPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
{
}

// ----------------------------------------------------------------------------
StreametaPlugin::~StreametaPlugin()
{
}

// ----------------------------------------------------------------------------
QWidget* StreametaPlugin::createView() 
{ 
    return new QWidget();
}

// ----------------------------------------------------------------------------
void StreametaPlugin::destroyView(QWidget* view) 
{ 
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

// ----------------------------------------------------------------------------
void StreametaPlugin::onGameSessionLoaded(rfcommon::Session* session) {}
void StreametaPlugin::onGameSessionUnloaded(rfcommon::Session* session) {}
void StreametaPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void StreametaPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

void StreametaPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void StreametaPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
