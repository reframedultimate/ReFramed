#include "ledge/PluginConfig.hpp"
#include "ledge/LedgePlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

static rfcommon::Plugin* createLedgePlugin(
    RFPluginFactory* factory, 
    rfcommon::PluginContext* pluginCtx, 
    rfcommon::Log* log, 
    rfcommon::UserMotionLabels* userLabels, 
    rfcommon::Hash40Strings* hash40Strings)
{
    PROFILE(PluginGlobal, createLedgePlugin);

    return new LedgePlugin(factory);
}

static void destroyLedgePlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyLedgePlugin);

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
    PROFILE(PluginGlobal, start);

    return 0;
}

static void stop()
{
    PROFILE(PluginGlobal, stop);

}

DEFINE_PLUGIN(factories, start, stop)
