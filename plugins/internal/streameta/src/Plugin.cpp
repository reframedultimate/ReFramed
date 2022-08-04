#include "streameta/PluginConfig.hpp"
#include "streameta/StreametaPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createStreametaPlugin(RFPluginFactory* factory)
{
    return new StreametaPlugin(factory);
}

static void destroyStreametaPlugin(rfcommon::Plugin* plugin)
{
    delete plugin;
}

static RFPluginFactory factories[] = {
    {createStreametaPlugin, destroyStreametaPlugin, RFPluginType::REALTIME,
    {"Streameta",
     "misc > misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "Integrate with the streameta platform"}},
     
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
