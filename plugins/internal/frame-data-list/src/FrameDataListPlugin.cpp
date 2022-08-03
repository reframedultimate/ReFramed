#include "frame-data-list/FrameDataListPlugin.hpp"
#include "frame-data-list/models/FrameDataListModel.hpp"
#include "frame-data-list/views/FrameDataListView.hpp"

// ----------------------------------------------------------------------------
FrameDataListPlugin::FrameDataListPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
    , model_(new FrameDataListModel)
{
}

// ----------------------------------------------------------------------------
FrameDataListPlugin::~FrameDataListPlugin()
{
}

// ----------------------------------------------------------------------------
QWidget* FrameDataListPlugin::createView()
{
    return new FrameDataListView(model_.get());
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    model_->finalizeSession(game);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    model_->finalizeSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) { (void)games; (void)numGames; }
void FrameDataListPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) { (void)games; (void)numGames; }

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void FrameDataListPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void FrameDataListPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void FrameDataListPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolTrainingStarted(rfcommon::Session* training)
{
    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    model_->setSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    // We probably want to clear the existing data in this case
    model_->finalizeSession(oldTraining);
    model_->setSession(newTraining);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
    model_->finalizeSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    model_->setSession(game);
}

// ----------------------------------------------------------------------------
void FrameDataListPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
    model_->finalizeSession(game);
}
