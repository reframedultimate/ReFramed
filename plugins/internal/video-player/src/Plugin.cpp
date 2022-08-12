#include "video-player/PluginConfig.hpp"
#include "video-player/VideoPlayerPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

#include <QWidget>

static rfcommon::Plugin* createVideoPlayerPlugin(RFPluginFactory* factory)
{
    return new VideoPlayerPlugin(factory);
}

static void destroyVideoPlayerPlugin(rfcommon::Plugin* plugin)
{
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
    return 0;
}

static void stop()
{
}

DEFINE_PLUGIN(factories, start, stop)
