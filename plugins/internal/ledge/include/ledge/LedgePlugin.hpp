#pragma once

#include "rfcommon/Plugin.hpp"
#include <memory>

class LedgePlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::RealtimeInterface
        , private rfcommon::Plugin::ReplayInterface
{
public:
    LedgePlugin(RFPluginFactory* factory);
    ~LedgePlugin();

private:
    QWidget* createView() override;
    void destroyView(QWidget* view) override;

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::SharedDataInterface* sharedInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;
    Plugin::VideoPlayerInterface* videoPlayerInterface() override final;

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
