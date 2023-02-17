#include "rfcommon/Profiler.hpp"
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
rfcommon::Plugin::SharedDataInterface* DamageTimePlotPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* DamageTimePlotPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* DamageTimePlotPlugin::createView()
{
    PROFILE(DamageTimePlotPlugin, createView);

    return new DamageTimePlotView(model_.get());
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::destroyView(QWidget* view)
{
    PROFILE(DamageTimePlotPlugin, destroyView);

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
    PROFILE(DamageTimePlotPlugin, onProtocolTrainingStarted);

    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    PROFILE(DamageTimePlotPlugin, onProtocolTrainingResumed);

    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    PROFILE(DamageTimePlotPlugin, onProtocolTrainingReset);

    model_->clearAll();
    model_->addSession(newTraining);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolTrainingEnded(rfcommon::Session* training)  {}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(DamageTimePlotPlugin, onProtocolGameStarted);

    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(DamageTimePlotPlugin, onProtocolGameResumed);

    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolGameEnded(rfcommon::Session* game) {}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    PROFILE(DamageTimePlotPlugin, onGameSessionLoaded);

    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    PROFILE(DamageTimePlotPlugin, onGameSessionUnloaded);

    model_->clearAll();
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    PROFILE(DamageTimePlotPlugin, onTrainingSessionLoaded);

    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    PROFILE(DamageTimePlotPlugin, onTrainingSessionUnloaded);

    model_->clearAll();
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    PROFILE(DamageTimePlotPlugin, onGameSessionSetLoaded);

    model_->clearAll();
    while (numGames--)
        model_->addSession(*games++);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    PROFILE(DamageTimePlotPlugin, onGameSessionSetUnloaded);

    model_->clearAll();
}
