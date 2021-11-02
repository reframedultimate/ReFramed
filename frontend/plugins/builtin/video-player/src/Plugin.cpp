#include "video-player/PluginConfig.hpp"
#include "video-player/VideoPlayer.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/PluginType.hpp"

#include <QWidget>

static uh::Plugin* createVideoPlayer()
{
    return new VideoPlayer;
}

static void destroy(uh::Plugin* plugin)
{
    delete plugin;
}

static UHPluginFactory factories[] = {
    {createVideoPlayer, destroy, UHPluginType::VISUALIZER,
     "Video Player", "TheComet", "alex.murray@gmx.ch", "A video player"},
    {NULL}
};

static int start(uint32_t version)
{
    return 0;
}

static void stop()
{
}

PLUGIN_API UHPluginInterface plugin_interface = {
    PLUGIN_VERSION,
    factories,
    start,
    stop
};
