#pragma once

#include "stats/listeners/SettingsListener.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Reference.hpp"
#include <memory>

namespace rfcommon {
    class FrameData;
    class Hash40Strings;
    class MappingInfo;
    class Metadata;
    class UserMotionLabels;
}

class PlayerMeta;
class StatsCalculator;
class SettingsModel;
class WebSocketServer;

class StatsPlugin
        : public rfcommon::Plugin
        , public rfcommon::Plugin::UIInterface
        , public rfcommon::Plugin::RealtimeInterface
        , public rfcommon::Plugin::ReplayInterface
        , public rfcommon::FrameDataListener
        , public SettingsListener
{
public:
    StatsPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
    ~StatsPlugin();

    void resetStatsIfAppropriate(rfcommon::Session* session);
    void clearSession();
    bool addSession(rfcommon::Session* session);

    /*!
     * \brief Export all files but fills them with dummy statistics (lots
     * of zeros, no names, etc.) This is mostly to indicate that a new game
     * has started and statistics aren't available yet.
     */
    void exportOBSEmptyStats() const;

    /*!
     * \brief Export all statistics to files.
     */
    void exportOBSStats() const;

    void sendWebSocketStats(bool gameStarted, bool gameEnded) const;

private:
    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::VisualizerInterface* visualizerInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;
    Plugin::VideoPlayerInterface* videoPlayerInterface() override final;

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

private:
    // These get called by the main application when connecting/disconnecting
    // to the Nintendo Switch. Typically you don't really need these,
    // but it might be interesting to show the status of the connection somewhere.
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    // These get called when a new game starts/ends, or if a new training mode session starts/ends.
    void onProtocolTrainingStarted(rfcommon::Session* training) override;
    void onProtocolTrainingResumed(rfcommon::Session* training) override;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::Session* training) override;
    void onProtocolGameStarted(rfcommon::Session* game) override;
    void onProtocolGameResumed(rfcommon::Session* game) override;
    void onProtocolGameEnded(rfcommon::Session* game) override;

private:
    // These get called when ReFramed loads/unloads a replay file
    void onGameSessionLoaded(rfcommon::Session* game) override;
    void onGameSessionUnloaded(rfcommon::Session* game) override;
    void onTrainingSessionLoaded(rfcommon::Session* training) override;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override;

    // These get called when the user selects multiple replay files in ReFramed and
    // loads them
    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override;

private:
    // These get called whenever a new frame is received (during an active game)
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    // The export code is implemented in these callbacks
    void onSettingsStatsChanged() override;
    void onSettingsOBSChanged() override;
    void onSettingsWSChanged() override;

private:
    rfcommon::Reference<rfcommon::FrameData> frameData_;
    std::unique_ptr<PlayerMeta> playerMeta_;
    std::unique_ptr<StatsCalculator> statsCalculator_;
    std::unique_ptr<SettingsModel> settingsModel_;
    std::unique_ptr<WebSocketServer> wsServer_;
    bool weAreLive_ = false;
};
