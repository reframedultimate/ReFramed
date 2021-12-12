#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/views/XYPositionsPlotView.hpp"
#include "xy-positions-plot/listeners/XYPositionsPlotListener.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/RunningTrainingSession.hpp"
#include "rfcommon/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
XYPositionsPlotModel::XYPositionsPlotModel(RFPluginFactory* factory)
    : RealtimePlugin(factory)
{
}

// ----------------------------------------------------------------------------
XYPositionsPlotModel::~XYPositionsPlotModel()
{
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::setSession(rfcommon::Session* session)
{
    session_ = session;
    session_->dispatcher.addListener(this);
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotSessionSet, session);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::clearSession(rfcommon::Session* session)
{
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotSessionCleared, session);
    session->dispatcher.removeListener(this);
    session_.drop();
}

// ----------------------------------------------------------------------------
QWidget* XYPositionsPlotModel::createView()
{
    return new XYPositionsPlotView(this);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    setSession(session);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    clearSession(session);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void XYPositionsPlotModel::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void XYPositionsPlotModel::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void XYPositionsPlotModel::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training)
{
    setSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training)
{
    setSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training)
{
    clearSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    clearSession(match);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name)
{
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotNameChanged, player, name);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state)
{
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotNewValue, player, state.posx(), state.posy());
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) { (void)number; }
void XYPositionsPlotModel::onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) { (void)number; }
void XYPositionsPlotModel::onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) { (void)format; }
void XYPositionsPlotModel::onRunningGameSessionWinnerChanged(int winner) { (void)winner; }
void XYPositionsPlotModel::onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) { (void)player; (void)state; }
