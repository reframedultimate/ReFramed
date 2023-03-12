#include "stage-stats/PluginConfig.hpp"
#include "stage-stats/StageStatsPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

static rfcommon::Plugin* createPlugin(
    RFPluginFactory* factory,
    rfcommon::PluginContext* pluginCtx,
    rfcommon::Log* log,
    rfcommon::MotionLabels* labels)
{
    PROFILE(PluginGlobal, createPlugin);

    return new StageStatsPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyPlugin);

    delete plugin;
}

static const RFPluginType pluginTypes =
    RFPluginType::UI |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, pluginTypes,
    {"Stage Stats",
     "misc > misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "Calculates some statistics relevnat to stage selection"}},

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
