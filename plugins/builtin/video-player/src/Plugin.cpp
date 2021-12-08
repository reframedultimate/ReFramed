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
static QWidget* createView(rfcommon::Plugin* model)
{
    VideoPlayer* videoPlayer = dynamic_cast<VideoPlayer*>(model);
    return videoPlayer;
}
static void destroyView(rfcommon::Plugin* model, QWidget* view)
{
}

static RFPluginFactory factories[] = {
    {createModel, destroyModel, createView, destroyView, RFPluginType::VISUALIZER,
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
