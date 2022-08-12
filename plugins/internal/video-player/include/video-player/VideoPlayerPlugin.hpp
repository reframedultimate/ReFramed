#pragma once

#include "rfcommon/Plugin.hpp"

class VideoPlayerPlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::ReplayInterface
        , private rfcommon::Plugin::VideoPlayerInterface
{
public:
    VideoPlayerPlugin(RFPluginFactory* factory);
    ~VideoPlayerPlugin();

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::VisualizerInterface* visualizerInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;
    Plugin::VideoPlayerInterface* videoPlayerInterface() override final;

private:
    QWidget* createView() override final;
    void destroyView(QWidget* view) override final;

private:
    void onGameSessionLoaded(rfcommon::Session* game) override final;
    void onGameSessionUnloaded(rfcommon::Session* game) override final;
    void onTrainingSessionLoaded(rfcommon::Session* training) override final;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override final;

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override final;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override final;
};
