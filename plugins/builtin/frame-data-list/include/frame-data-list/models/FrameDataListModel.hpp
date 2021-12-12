#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class Session;
}

class FrameDataListListener;

class FrameDataListModel : public rfcommon::RealtimePlugin
{
public:
    FrameDataListModel(RFPluginFactory* factory);
    ~FrameDataListModel();

    rfcommon::Session* session() const
        { return session_; }

    rfcommon::ListenerDispatcher<FrameDataListListener> dispatcher;

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
    void onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training) override;
    void onProtocolMatchStarted(rfcommon::RunningGameSession* match) override;
    void onProtocolMatchResumed(rfcommon::RunningGameSession* match) override;
    void onProtocolMatchEnded(rfcommon::RunningGameSession* match) override;

private:
    rfcommon::Reference<rfcommon::Session> session_;
};
