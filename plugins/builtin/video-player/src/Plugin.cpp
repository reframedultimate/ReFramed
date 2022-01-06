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
    {"Video Player",
     "misc",
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
