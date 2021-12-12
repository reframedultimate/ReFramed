#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class Session;
}

class DamageTimePlotListener;

class DamageTimePlotModel : public rfcommon::RealtimePlugin
                          , public rfcommon::SessionListener
{
public:
    DamageTimePlotModel(RFPluginFactory* factory);
    ~DamageTimePlotModel();

    const rfcommon::Session* session() const
        { return session_; }

    rfcommon::ListenerDispatcher<DamageTimePlotListener> dispatcher;

private:
    QWidget* createView() override;
    void destroyView(QWidget* view) override;

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training) override;
    void onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training) override;
    void onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training) override;
    void onProtocolMatchStarted(rfcommon::RunningGameSession* match) override;
    void onProtocolMatchResumed(rfcommon::RunningGameSession* match) override;
    void onProtocolMatchEnded(rfcommon::RunningGameSession* match) override;

private:
    void onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state) override;

    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override;
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override;
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) override;
    void onRunningGameSessionWinnerChanged(int winner) override;
    void onRunningTrainingSessionTrainingReset() override;
    void onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) override;

private:
    rfcommon::Reference<rfcommon::Session> session_;
};
