#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfcommon/FighterFrame.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
DamageTimePlotModel::DamageTimePlotModel(RFPluginFactory* factory)
    : RealtimePlugin(factory)
{
}

// ----------------------------------------------------------------------------
DamageTimePlotModel::~DamageTimePlotModel()
{
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::setSession(rfcommon::Session* session)
{
    session_ = session;
    session_->dispatcher.addListener(this);
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotSessionSet, session);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::clearSession(rfcommon::Session* session)
{
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotSessionCleared, session);
    session->dispatcher.removeListener(this);
    session_.drop();
}

// ----------------------------------------------------------------------------
QWidget* DamageTimePlotModel::createView()
{
    return new DamageTimePlotView(this);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    setSession(session);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    clearSession(session);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DamageTimePlotModel::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void DamageTimePlotModel::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DamageTimePlotModel::onProtocolDisconnectedFromServer() {}
void DamageTimePlotModel::onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training) { (void)training; }
void DamageTimePlotModel::onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training) { (void)training; }
void DamageTimePlotModel::onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldTraining, rfcommon::RunningTrainingSession* newTraining) { (void)oldTraining; (void)newTraining; }
void DamageTimePlotModel::onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training) { (void)training; }

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    clearSession(match);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningGameSessionPlayerNameChanged(int playerIdx, const rfcommon::SmallString<15>& name)
{
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotNameChanged, playerIdx, name);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningSessionNewUniquePlayerState(int playerIdx, const rfcommon::FighterFrame& state)
{
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotNewValue, playerIdx, state.frame(), state.damage());
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) { (void)number; }
void DamageTimePlotModel::onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) { (void)number; }
void DamageTimePlotModel::onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) { (void)format; }
void DamageTimePlotModel::onRunningGameSessionWinnerChanged(int winner) { (void)winner; }
void DamageTimePlotModel::onRunningSessionNewPlayerState(int player, const rfcommon::FighterFrame& state) { (void)player; (void)state; }
void DamageTimePlotModel::onRunningSessionNewUniqueFrame(const rfcommon::SmallVector<rfcommon::FighterFrame, 8>& states) { (void)states; }
void DamageTimePlotModel::onRunningSessionNewFrame(const rfcommon::SmallVector<rfcommon::FighterFrame, 8>& states) { (void)states; }
