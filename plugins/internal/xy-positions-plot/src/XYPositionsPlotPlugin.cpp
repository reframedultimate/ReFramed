#include "xy-positions-plot/XYPositionsPlotPlugin.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/views/XYPositionsPlotView.hpp"

// ----------------------------------------------------------------------------
XYPositionsPlotPlugin::XYPositionsPlotPlugin(RFPluginFactory* factory)
    : rfcommon::RealtimePlugin(factory)
    , model_(new XYPositionsPlotModel)
{
}

// ----------------------------------------------------------------------------
XYPositionsPlotPlugin::~XYPositionsPlotPlugin()
{
}

// ----------------------------------------------------------------------------
QWidget* XYPositionsPlotPlugin::createView()
{
    return new XYPositionsPlotView(model_.get());
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void XYPositionsPlotPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void XYPositionsPlotPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void XYPositionsPlotPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingStarted(rfcommon::Session* training)
{
    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingResumed(rfcommon::Session* training) 
{
    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    model_->clearAll();
    model_->addSession(newTraining);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
    // Keep data around until next session
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    model_->clearAll();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    model_->clearAll();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) 
{
    model_->clearAll();
    for (int i = 0; i != numGames; ++i)
        model_->addSession(games[i]);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) 
{
    model_->clearAll();
}
