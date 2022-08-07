#include "data-viewer/PluginConfig.hpp"
#include "data-viewer/DataViewerPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createPlugin(RFPluginFactory* factory)
{
    return new DataViewerPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* model)
{
    delete model;
}

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, RFPluginType::REALTIME, {
         "Data Viewer",
         "misc",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "View raw data in replay files such as metadata, mapping info, and frame data"
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
