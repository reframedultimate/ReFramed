#include "video-player/PluginConfig.hpp"
#include "video-player/VideoPlayerPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

#include <QWidget>

static rfcommon::Plugin* createVideoPlayerPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings, rfcommon::Log* log)
{
    PROFILE(PluginGlobal, createVideoPlayerPlugin);

    return new VideoPlayerPlugin(factory, log);
}

static void destroyVideoPlayerPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyVideoPlayerPlugin);

    delete plugin;
}

static const RFPluginType videoPlayerPluginTypes =
    RFPluginType::UI |
    RFPluginType::REPLAY |
    RFPluginType::VIDEO_PLAYER;

static RFPluginFactory factories[] = {
    {createVideoPlayerPlugin, destroyVideoPlayerPlugin, videoPlayerPluginTypes, {
         "Video Player",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "A video player"}},

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
