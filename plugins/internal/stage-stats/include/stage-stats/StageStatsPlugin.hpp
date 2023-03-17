#pragma once

#include "rfcommon/Plugin.hpp"
#include <memory>

class StageStatsModel;

class StageStatsPlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::ReplayInterface
{
public:
    StageStatsPlugin(RFPluginFactory* factory);
    ~StageStatsPlugin();

private:
    QWidget* createView() override;
    void destroyView(QWidget* view) override;

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::SharedDataInterface* sharedInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;
    Plugin::VideoPlayerInterface* videoPlayerInterface() override final;

private:
    void onGameSessionLoaded(rfcommon::Session* session) override;
    void onGameSessionUnloaded(rfcommon::Session* session) override;
    void onTrainingSessionLoaded(rfcommon::Session* training) override;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override;

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override;

private:
    std::unique_ptr<StageStatsModel> model_;
};
