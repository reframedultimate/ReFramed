#include "ledge/PluginConfig.hpp"
#include "ledge/LedgePlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createLedgePlugin(RFPluginFactory* factory)
{
    return new LedgePlugin(factory);
}

static void destroyLedgePlugin(rfcommon::Plugin* plugin)
{
    delete plugin;
}

static const RFPluginType ledgePluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createLedgePlugin, destroyLedgePlugin, ledgePluginTypes,
    {"Ledge",
     "misc > misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "Analyzes options and timings chosen from ledge"}},
     
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
