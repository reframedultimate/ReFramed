#pragma once

#include "rfcommon/Plugin.hpp"
#include <memory>

class StreametaPlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::RealtimeInterface
{
public:
    StreametaPlugin(RFPluginFactory* factory);
    ~StreametaPlugin();

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::VisualizerInterface* visualizerInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;
    Plugin::VideoPlayerInterface* videoPlayerInterface() override final;

private:
    QWidget* createView() override final;
    void destroyView(QWidget* view) override final;

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override final;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override final;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override final;
    void onProtocolDisconnectedFromServer() override final;

    void onProtocolTrainingStarted(rfcommon::Session* training) override final;
    void onProtocolTrainingResumed(rfcommon::Session* training) override final;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override final;
    void onProtocolTrainingEnded(rfcommon::Session* training) override final;
    void onProtocolGameStarted(rfcommon::Session* match) override final;
    void onProtocolGameResumed(rfcommon::Session* match) override final;
    void onProtocolGameEnded(rfcommon::Session* match) override final;

private:
};
