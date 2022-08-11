#include "damage-time-plot/PluginConfig.hpp"
#include "damage-time-plot/DamageTimePlotPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createPlugin(RFPluginFactory* factory)
{
    return new DamageTimePlotPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* plugin)
{
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
    return 0;
}

static void stop()
{
}

DEFINE_PLUGIN(factories, start, stop)
