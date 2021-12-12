#include "video-player/PluginConfig.hpp"
#include "video-player/VideoPlayer.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/PluginType.hpp"

#include <QWidget>

static rfcommon::Plugin* createModel(RFPluginFactory* factory)
{
    return new VideoPlayer(factory);
}
static void destroyModel(rfcommon::Plugin* model)
{
    delete model;
}

static RFPluginFactory factories[] = {
    {createModel, destroyModel, RFPluginType::VISUALIZER,
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

DEFINE_PLUGIN(factories, start, stop)
