#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/Reference.hpp"
#include <memory>

class AVDecoder;
class BufferedSeekableDecoder;
class VideoPlayerModel;

namespace rfcommon {
    class Log;
    class VideoEmbed;
}

class VideoPlayerPlugin
        : public rfcommon::Plugin
        , private rfcommon::Plugin::UIInterface
        , private rfcommon::Plugin::ReplayInterface
{
public:
    VideoPlayerPlugin(RFPluginFactory* factory, rfcommon::Log* log);
    ~VideoPlayerPlugin();

    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::SharedDataInterface* sharedInterface() override final;
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

private:
    rfcommon::Log* log_;
    std::unique_ptr<AVDecoder> decoder_;
    std::unique_ptr<BufferedSeekableDecoder> seekableDecoder_;
    std::unique_ptr<VideoPlayerModel> videoPlayer_;
    rfcommon::Reference<rfcommon::VideoEmbed> activeVideo_;
};
