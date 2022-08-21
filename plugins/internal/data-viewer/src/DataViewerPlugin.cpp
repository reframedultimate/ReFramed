#include "rfcommon/Profiler.hpp"
#include "data-viewer/DataViewerPlugin.hpp"
#include "data-viewer/models/DataViewerModel.hpp"
#include "data-viewer/views/DataViewerView.hpp"

// ----------------------------------------------------------------------------
DataViewerPlugin::DataViewerPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : Plugin(factory)
    , model_(new DataViewerModel(userLabels, hash40Strings))
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
rfcommon::Plugin::VideoPlayerInterface* DataViewerPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* DataViewerPlugin::createView()
{
    PROFILE(DataViewerPlugin, createView);

    return new DataViewerView(model_.get());
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::destroyView(QWidget* view)
{
    PROFILE(DataViewerPlugin, destroyView);

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
    PROFILE(DataViewerPlugin, onProtocolTrainingStarted);

    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    PROFILE(DataViewerPlugin, onProtocolTrainingResumed);

    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    PROFILE(DataViewerPlugin, onProtocolTrainingReset);

    model_->setSession(newTraining);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
    PROFILE(DataViewerPlugin, onProtocolTrainingEnded);

    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(DataViewerPlugin, onProtocolGameStarted);

    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(DataViewerPlugin, onProtocolGameResumed);

    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
    PROFILE(DataViewerPlugin, onProtocolGameEnded);

}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    PROFILE(DataViewerPlugin, onGameSessionLoaded);

    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    PROFILE(DataViewerPlugin, onGameSessionUnloaded);

    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    PROFILE(DataViewerPlugin, onTrainingSessionLoaded);

    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    PROFILE(DataViewerPlugin, onTrainingSessionUnloaded);

    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    PROFILE(DataViewerPlugin, onGameSessionSetLoaded);

    model_->clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    PROFILE(DataViewerPlugin, onGameSessionSetUnloaded);

    (void)games;
    (void)numGames;
}
