#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class Session;
}

class XYPositionsPlotListener;

class XYPositionsPlotModel : public rfcommon::RealtimePlugin
                           , public rfcommon::SessionListener
{
public:
    XYPositionsPlotModel(RFPluginFactory* factory);
    ~XYPositionsPlotModel();

    rfcommon::Session* session() const
        { return session_; }

    rfcommon::ListenerDispatcher<XYPositionsPlotListener> dispatcher;

private:
    void setSession(rfcommon::Session* session);
    void clearSession(rfcommon::Session* session);

private:
    QWidget* createView() override;
    void destroyView(QWidget* view) override;

private:
    void setSavedGameSession(rfcommon::SavedGameSession* session) override;
    void clearSavedGameSession(rfcommon::SavedGameSession* session) override;

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training) override;
    void onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training) override;
    void onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldTraining, rfcommon::RunningTrainingSession* newTraining) override;
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
    void onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) override;
    void onRunningSessionNewUniqueFrame(const rfcommon::SmallVector<rfcommon::PlayerState, 8>& states) override;
    void onRunningSessionNewFrame(const rfcommon::SmallVector<rfcommon::PlayerState, 8>& states) override;

private:
    rfcommon::Reference<rfcommon::Session> session_;
};
