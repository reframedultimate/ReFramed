#pragma once

#include "overextension/listeners/OverextensionListener.hpp"
#include "rfcommon/Plugin.hpp"
#include <memory>

class OverextensionModel;

namespace rfcommon {
    class UserMotionLabels;
}

class OverextensionPlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::RealtimeInterface
        , private rfcommon::Plugin::ReplayInterface
        , private rfcommon::Plugin::SharedDataInterface
        , private OverextensionListener
{
public:
    OverextensionPlugin(RFPluginFactory* factory, rfcommon::PluginContext* pluginCtx, rfcommon::UserMotionLabels* userLabels);
    ~OverextensionPlugin();

private:
    QWidget* createView() override;
    void destroyView(QWidget* view) override;

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::SharedDataInterface* visualizerInterface() override final;
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
    void onSharedDataChanged() override;

private:
    void onPlayersChanged() override;
    void onDataChanged() override;
    void onCurrentFighterChanged(int fighterIdx) override;
    void exportTimeIntervals();

private:
    std::unique_ptr<OverextensionModel> model_;
    rfcommon::UserMotionLabels* userLabels_;
};
