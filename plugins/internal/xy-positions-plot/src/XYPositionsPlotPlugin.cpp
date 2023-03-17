#include "rfcommon/Profiler.hpp"
#include "xy-positions-plot/XYPositionsPlotPlugin.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/views/XYPositionsPlotView.hpp"

// ----------------------------------------------------------------------------
XYPositionsPlotPlugin::XYPositionsPlotPlugin(RFPluginFactory* factory)
    : Plugin(factory)
    , model_(new XYPositionsPlotModel)
{
}

// ----------------------------------------------------------------------------
XYPositionsPlotPlugin::~XYPositionsPlotPlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* XYPositionsPlotPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* XYPositionsPlotPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* XYPositionsPlotPlugin::replayInterface() { return this; }
rfcommon::Plugin::SharedDataInterface* XYPositionsPlotPlugin::sharedInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* XYPositionsPlotPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* XYPositionsPlotPlugin::createView()
{
    PROFILE(XYPositionsPlotPlugin, createView);

    return new XYPositionsPlotView(model_.get());
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::destroyView(QWidget* view)
{
    PROFILE(XYPositionsPlotPlugin, destroyView);

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
    PROFILE(XYPositionsPlotPlugin, onProtocolTrainingStarted);

    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    PROFILE(XYPositionsPlotPlugin, onProtocolTrainingResumed);

    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    PROFILE(XYPositionsPlotPlugin, onProtocolTrainingReset);

    model_->clearAll();
    model_->addSession(newTraining);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
    PROFILE(XYPositionsPlotPlugin, onProtocolTrainingEnded);

}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(XYPositionsPlotPlugin, onProtocolGameStarted);

    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(XYPositionsPlotPlugin, onProtocolGameResumed);

    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
    PROFILE(XYPositionsPlotPlugin, onProtocolGameEnded);

    // Keep data around until next session
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    PROFILE(XYPositionsPlotPlugin, onGameSessionLoaded);

    model_->clearAll();
    model_->addSession(game);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    PROFILE(XYPositionsPlotPlugin, onGameSessionUnloaded);

    model_->clearAll();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    PROFILE(XYPositionsPlotPlugin, onTrainingSessionLoaded);

    model_->clearAll();
    model_->addSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    PROFILE(XYPositionsPlotPlugin, onTrainingSessionUnloaded);

    model_->clearAll();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    PROFILE(XYPositionsPlotPlugin, onGameSessionSetLoaded);

    model_->clearAll();
    for (int i = 0; i != numGames; ++i)
        model_->addSession(games[i]);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    PROFILE(XYPositionsPlotPlugin, onGameSessionSetUnloaded);

    model_->clearAll();
}
