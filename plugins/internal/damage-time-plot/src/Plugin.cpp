#include "damage-time-plot/PluginConfig.hpp"
#include "damage-time-plot/DamageTimePlotPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

static rfcommon::Plugin* createPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
{
    PROFILE(PluginGlobal, createPlugin);

    return new DamageTimePlotPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyPlugin);

    delete plugin;
}

static const RFPluginType damageTimePluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, damageTimePluginTypes, {
        "Damage vs Time Plot",
        "misc",
        "TheComet",
        "TheComet#5387, @TheComet93",
        "Plots each player's damage over time"
    }},

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
