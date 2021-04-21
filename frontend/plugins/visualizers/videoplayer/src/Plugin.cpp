#include "videoplayer/PluginConfig.hpp"
#include "videoplayer/VideoPlayer.hpp"
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

static PluginFactory factories[] = {
    {createVideoPlayer, destroy, uh::PluginType::VISUALIZER,
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

PLUGIN_API PluginInterface plugin_interface = {
    start,
    stop,
    factories
};
