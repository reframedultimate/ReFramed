#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include <memory>

class StreametaPlugin : public rfcommon::RealtimePlugin
{
public:
    StreametaPlugin(RFPluginFactory* factory);
    ~StreametaPlugin();

private:
    QWidget* createView() override;
    void destroyView(QWidget* view) override;

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::Session* training) override;
    void onProtocolTrainingResumed(rfcommon::Session* training) override;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::Session* training) override;
    void onProtocolGameStarted(rfcommon::Session* match) override;
    void onProtocolGameResumed(rfcommon::Session* match) override;
    void onProtocolGameEnded(rfcommon::Session* match) override;

private:
    void onGameSessionLoaded(rfcommon::Session* session) override;
    void onGameSessionUnloaded(rfcommon::Session* session) override;
    void onTrainingSessionLoaded(rfcommon::Session* training) override;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override;

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override;

private:
};
