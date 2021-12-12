#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/RunningGameSession.hpp"

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
void DamageTimePlotModel::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DamageTimePlotModel::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void DamageTimePlotModel::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void DamageTimePlotModel::onProtocolDisconnectedFromServer() {}
void DamageTimePlotModel::onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training) { (void)training; }
void DamageTimePlotModel::onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training) { (void)training; }
void DamageTimePlotModel::onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training) { (void)training; }

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    match->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
    match->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    match->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name)
{
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotNameChanged, player, name);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state)
{
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotNewValue, player, state.frame(), state.damage());
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) { (void)number; }
void DamageTimePlotModel::onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) { (void)number; }
void DamageTimePlotModel::onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) { (void)format; }
void DamageTimePlotModel::onRunningGameSessionWinnerChanged(int winner) { (void)winner; }
void DamageTimePlotModel::onRunningTrainingSessionTrainingReset() {}
void DamageTimePlotModel::onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) { (void)player; (void)state; }

