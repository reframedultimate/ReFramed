#include "data-viewer/DataViewerPlugin.hpp"
#include "data-viewer/models/DataViewerModel.hpp"
#include "data-viewer/views/DataViewerView.hpp"

// ----------------------------------------------------------------------------
DataViewerPlugin::DataViewerPlugin(RFPluginFactory* factory)
    : Plugin(factory)
    , model_(new DataViewerModel)
{
}

// ----------------------------------------------------------------------------
DataViewerPlugin::~DataViewerPlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* DataViewerPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* DataViewerPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* DataViewerPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* DataViewerPlugin::visualizerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* DataViewerPlugin::createView()
{
    return new DataViewerView(model_.get());
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DataViewerPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void DataViewerPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DataViewerPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingStarted(rfcommon::Session* training)
{
    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    model_->setSession(newTraining);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    (void)games;
    (void)numGames;
}
