#include "vod-review/PluginConfig.hpp"
#include "vod-review/VideoPlayerPlugin.hpp"
#include "vod-review/VODReviewPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

#include <QWidget>

static rfcommon::Plugin* createVideoPlayerPlugin(
        RFPluginFactory* factory,
        rfcommon::PluginContext* pluginCtx,
        rfcommon::Log* log,
        rfcommon::MotionLabels* labels)
{
    PROFILE(PluginGlobal, createVideoPlayerPlugin);

    return new VideoPlayerPlugin(factory, log);
}

static rfcommon::Plugin* createVODReviewPlugin(
        RFPluginFactory* factory,
        rfcommon::PluginContext* pluginCtx,
        rfcommon::Log* log,
        rfcommon::MotionLabels* labels)
{
    PROFILE(PluginGlobal, createVODReviewPlugin);

    return new VODReviewPlugin(factory, pluginCtx, log);
}

static void destroyPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyPlugin);

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
