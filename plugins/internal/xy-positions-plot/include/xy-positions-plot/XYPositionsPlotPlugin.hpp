#pragma once

#include "rfcommon/Plugin.hpp"
#include <memory>

class XYPositionsPlotModel;

class XYPositionsPlotPlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::RealtimeInterface
        , private rfcommon::Plugin::ReplayInterface
{
public:
    XYPositionsPlotPlugin(RFPluginFactory* factory);
    ~XYPositionsPlotPlugin();

private:
    /*!
     * This is called by ReFramed to create an instance of your view.
     * It is possible that this gets called more than once, for example if
     * ReFramed wants to add your view to different parts of the program.
     * Your view should be designed in a way such that multiple views can
     * share the same underlying model.
     */
    QWidget* createView() override final;

    /*!
     * The counter-part to createView(). When ReFramed removes your view
     * it will give it back to you to destroy.
     */
    void destroyView(QWidget* view) override final;

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::VisualizerInterface* visualizerInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;

private:
    // These get called by the main application when connecting/disconnecting
    // to the Nintendo Switch.
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override final;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override final;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override final;
    void onProtocolDisconnectedFromServer() override final;

    // These get called when a new game starts/ends, or if a new training mode session starts/ends.
    void onProtocolTrainingStarted(rfcommon::Session* training) override final;
    void onProtocolTrainingResumed(rfcommon::Session* training) override final;
    void onProtocolTrainingReset(rfcommon::Session* training, rfcommon::Session* newSession) override final;
    void onProtocolTrainingEnded(rfcommon::Session* training) override final;
    void onProtocolGameStarted(rfcommon::Session* game) override final;
    void onProtocolGameResumed(rfcommon::Session* game) override final;
    void onProtocolGameEnded(rfcommon::Session* game) override final;

private:
    // These get called when ReFramed loads/unloads a replay file
    void onGameSessionLoaded(rfcommon::Session* game) override final;
    void onGameSessionUnloaded(rfcommon::Session* game) override final;
    void onTrainingSessionLoaded(rfcommon::Session* training) override final;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override final;

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override final;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override final;

private:
    std::unique_ptr<XYPositionsPlotModel> model_;
};
