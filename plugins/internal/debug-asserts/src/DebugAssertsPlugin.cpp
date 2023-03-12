#include "debug-asserts/PluginConfig.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class DebugAssertsPlugin
    : public rfcommon::Plugin
    , public rfcommon::Plugin::UIInterface
    , public rfcommon::Plugin::RealtimeInterface
    , public rfcommon::Plugin::ReplayInterface
{
public:
    DebugAssertsPlugin(RFPluginFactory* factory) : Plugin(factory) {}
    ~DebugAssertsPlugin() {
        assert(sessionState_ == NONE);
        assert(connectState_ == DISCONNECTED);
    }

    enum ConnectState
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    enum SessionState
    {
        NONE,
        LIVE_TRAINING,
        LIVE_GAME,
        REPLAY_TRAINING,
        REPLAY_GAME,
        REPLAY_GAME_SET
    };

private:
    Plugin::UIInterface* uiInterface() override { return this; }
    Plugin::ReplayInterface* replayInterface() override { return this; }
    Plugin::SharedDataInterface* visualizerInterface() override { return nullptr; }
    Plugin::RealtimeInterface* realtimeInterface() override { return this; }
    Plugin::VideoPlayerInterface* videoPlayerInterface() override { return nullptr; }

private:
    QWidget* createView() override {
        return widgets_.emplace(new QWidget);
    }
    void destroyView(QWidget* view) override {
        auto it = widgets_.findFirst(view);
        assert(*it == view);
        widgets_.erase(it);
        delete view;
    }

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override {
        assert(connectState_ == DISCONNECTED);
        assert(sessionState_ != LIVE_TRAINING && sessionState_ != LIVE_GAME);
        connectState_ = CONNECTING;
    }
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override {
        assert(connectState_ == CONNECTING);
        assert(sessionState_ != LIVE_TRAINING && sessionState_ != LIVE_GAME);
        connectState_ = DISCONNECTED;
    }
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override {
        assert(connectState_ == CONNECTING);
        assert(sessionState_ != LIVE_TRAINING && sessionState_ != LIVE_GAME);
        connectState_ = CONNECTED;
    }
    void onProtocolDisconnectedFromServer() override {
        assert(connectState_ == CONNECTED);
        assert(sessionState_ != LIVE_TRAINING && sessionState_ != LIVE_GAME);
        connectState_ = DISCONNECTED;
    }

    void onProtocolTrainingStarted(rfcommon::Session* training) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        sessions_.push(training);
        sessionState_ = LIVE_TRAINING;
    }
    void onProtocolTrainingResumed(rfcommon::Session* training) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        sessions_.push(training);
        sessionState_ = LIVE_TRAINING;
    }
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 1);
        assert(sessions_[0] == oldTraining);
        assert(sessionState_ == LIVE_TRAINING);
        sessions_[0] = newTraining;
    }
    void onProtocolTrainingEnded(rfcommon::Session* training) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 1);
        assert(sessions_[0] == training);
        assert(sessionState_ == LIVE_TRAINING);
        sessions_.pop();
        sessionState_ = NONE;
    }
    void onProtocolGameStarted(rfcommon::Session* game) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        sessions_.push(game);
        sessionState_ = LIVE_GAME;
    }
    void onProtocolGameResumed(rfcommon::Session* game) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        sessions_.push(game);
        sessionState_ = LIVE_GAME;
    }
    void onProtocolGameEnded(rfcommon::Session* game) override {
        assert(connectState_ == CONNECTED);
        assert(sessions_.count() == 1);
        assert(sessions_[0] == game);
        assert(sessionState_ = LIVE_GAME);
        sessions_.pop();
        sessionState_ = NONE;
    }

private:
    void onGameSessionLoaded(rfcommon::Session* game) override {
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        sessions_.push(game);
        sessionState_ = REPLAY_GAME;
    }
    void onGameSessionUnloaded(rfcommon::Session* game) override {
        assert(sessions_.count() == 1);
        assert(sessions_[0] == game);
        assert(sessionState_ == REPLAY_GAME);
        sessions_.pop();
        sessionState_ = NONE;
    }
    void onTrainingSessionLoaded(rfcommon::Session* training) override {
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        sessions_.push(training);
        sessionState_ = REPLAY_TRAINING;
    }
    void onTrainingSessionUnloaded(rfcommon::Session* training) override {
        assert(sessions_.count() == 1);
        assert(sessions_[0] == training);
        assert(sessionState_ == REPLAY_TRAINING);
        sessions_.pop();
        sessionState_ = NONE;
    }

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override {
        assert(sessions_.count() == 0);
        assert(sessionState_ == NONE);
        for (int i = 0; i != numGames; ++i)
            sessions_.push(games[i]);
        sessionState_ = REPLAY_GAME_SET;
    }
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override {
        assert(sessions_.count() == numGames);
        for (int i = 0; i != numGames; ++i)
            assert(sessions_[i] == games[i]);
        assert(sessionState_ == REPLAY_GAME_SET);
        sessions_.clear();
        sessionState_ = NONE;
    }

private:
    rfcommon::Vector<QWidget*> widgets_;
    rfcommon::Vector<rfcommon::Session*> sessions_;

    ConnectState connectState_ = DISCONNECTED;
    SessionState sessionState_ = NONE;
};

static rfcommon::Plugin* createPlugin(
        RFPluginFactory* factory,
        rfcommon::PluginContext* pluginCtx,
        rfcommon::Log* log,
        rfcommon::MotionLabels* labels)
{
    PROFILE(DebugAssertsPluginGlobal, createPlugin);
    return new DebugAssertsPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(DebugAssertsPluginGlobal, destroyPlugin);
    delete plugin;
}

static const RFPluginType pluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, pluginTypes,
    {"Debug Assertions",
     "misc > misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "Used to debug bugs in the plugin events system"}},

    {nullptr}
};

static int start(uint32_t version, const char** error)
{
    PROFILE(DebugAssertsPluginGlobal, start);

    return 0;
}

static void stop()
{
    PROFILE(DebugAssertsPluginGlobal, stop);

}

DEFINE_PLUGIN(factories, start, stop)
