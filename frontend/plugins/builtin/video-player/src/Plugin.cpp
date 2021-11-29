#include "video-player/PluginConfig.hpp"
#include "video-player/VideoPlayer.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/PluginType.hpp"

#include <QWidget>

static uh::Plugin* createModel(UHPluginFactory* factory)
{
    return new VideoPlayer(factory);
}
static void destroyModel(uh::Plugin* model)
{
    delete model;
}
static QWidget* createView(uh::Plugin* model)
{
    VideoPlayer* videoPlayer = dynamic_cast<VideoPlayer*>(model);
    return videoPlayer;
}
static void destroyView(uh::Plugin* model, QWidget* view)
{
}

static UHPluginFactory factories[] = {
    {createModel, destroyModel, createView, destroyView, UHPluginType::VISUALIZER,
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
