#include "video-player/PluginConfig.hpp"
#include "video-player/VideoPlayerPlugin.hpp"
#include "video-player/VODReviewPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

#include <QWidget>

static rfcommon::Plugin* createVideoPlayerPlugin(
        RFPluginFactory* factory,
        rfcommon::UserMotionLabels* userLabels,
        rfcommon::Hash40Strings* hash40Strings,
        rfcommon::Log* log)
{
    PROFILE(PluginGlobal, createVideoPlayerPlugin);

    return new VideoPlayerPlugin(factory, log);
}

static rfcommon::Plugin* createVODReviewPlugin(
        RFPluginFactory* factory,
        rfcommon::UserMotionLabels* userLabels,
        rfcommon::Hash40Strings* hash40Strings,
        rfcommon::Log* log)
{
    PROFILE(PluginGlobal, createVideoPlayerPlugin);

    return new VODReviewPlugin(factory, log);
}

static void destroyPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyVideoPlayerPlugin);

    delete plugin;
}

static const RFPluginType videoPlayerPluginTypes =
        RFPluginType::UI |
        RFPluginType::VIDEO_PLAYER;

static const RFPluginType vodReviewPluginTypes =
        RFPluginType::UI |
        RFPluginType::REPLAY |
        RFPluginType::VISUALIZER;

static RFPluginFactory factories[] = {
    {createVideoPlayerPlugin, destroyPlugin, videoPlayerPluginTypes, {
         "Video Player",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "A video player"}},

    {createVODReviewPlugin, destroyPlugin, vodReviewPluginTypes, {
         "VOD Review",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "A tool for reviewing VODs"}},

    {nullptr}
};

static int start(uint32_t version, const char** error)
{
    PROFILE(PluginGlobal, start);

    return 0;
}

static void stop()
{
    PROFILE(PluginGlobal, stop);

}

DEFINE_PLUGIN(factories, start, stop)
