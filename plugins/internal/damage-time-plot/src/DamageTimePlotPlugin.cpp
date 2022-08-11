#include "damage-time-plot/DamageTimePlotPlugin.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"

// ----------------------------------------------------------------------------
DamageTimePlotPlugin::DamageTimePlotPlugin(RFPluginFactory* factory)
    : Plugin(factory)
    , model_(new DamageTimePlotModel)
{
}

// ----------------------------------------------------------------------------
DamageTimePlotPlugin::~DamageTimePlotPlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* DamageTimePlotPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* DamageTimePlotPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* DamageTimePlotPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* DamageTimePlotPlugin::visualizerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* DamageTimePlotPlugin::createView()
{
    return new DamageTimePlotView(model_.get());
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DamageTimePlotPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void DamageTimePlotPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DamageTimePlotPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingStarted(rfcommon::Session* training) 
{
    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingResumed(rfcommon::Session* training) 
{
    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) 
{
    model_->clearAll();
    model_->addSession(newTraining);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingEnded(rfcommon::Session* training)  {}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolGameEnded(rfcommon::Session* game) {}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    model_->clearAll();
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    model_->clearAll();
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) 
{
    model_->clearAll();
    while (numGames--)
        model_->addSession(*games++);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) 
{
    model_->clearAll();
}
