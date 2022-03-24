#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include <memory>

class DamageTimePlotModel;

class DamageTimePlotPlugin : public rfcommon::RealtimePlugin
{
public:
    DamageTimePlotPlugin(RFPluginFactory* factory);
    ~DamageTimePlotPlugin();

private:
    /*!
     * This is called by ReFramed to create an instance of your view.
     * It is possible that this gets called more than once, for example if
     * ReFramed wants to add your view to different parts of the program.
     * Your view should be designed in a way such that multiple views can
     * share the same underlying model.
     */
    QWidget* createView() override;

    /*!
     * The counter-part to createView(). When ReFramed removes your view
     * it will give it back to you to destroy.
     */
    void destroyView(QWidget* view) override;

    // These get called by the main application when connecting/disconnecting
    // to the Nintendo Switch.
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override {}
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolDisconnectedFromServer() override {}

    // These get called when a new game starts/ends, or if a new training mode session starts/ends.
    void onProtocolTrainingStarted(rfcommon::RunningTrainingSession* session) override {}
    void onProtocolTrainingResumed(rfcommon::RunningTrainingSession* session) override {}
    void onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldSession, rfcommon::RunningTrainingSession* newSession) override {}
    void onProtocolTrainingEnded(rfcommon::RunningTrainingSession* session) override {}
    void onProtocolMatchStarted(rfcommon::RunningGameSession* session) override;
    void onProtocolMatchResumed(rfcommon::RunningGameSession* session) override;
    void onProtocolMatchEnded(rfcommon::RunningGameSession* session) override;

    // These get called when ReFramed loads/unloads a replay file
    void setSavedGameSession(rfcommon::SavedGameSession* session) override;
    void clearSavedGameSession(rfcommon::SavedGameSession* session) override;

private:
    std::unique_ptr<DamageTimePlotModel> model_;
};
