#include "frame-data-list/PluginConfig.hpp"
#include "frame-data-list/FrameDataListPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createPlugin(RFPluginFactory* factory)
{
    return new FrameDataListPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* model)
{
    delete model;
}

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, RFPluginType::REALTIME, {
         "Frame Data List",
         "misc",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "View the data of each frame"
     }},

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
