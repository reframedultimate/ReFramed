#include "damage-time-plot/PluginConfig.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createModel(RFPluginFactory* factory)
{
    return new DamageTimePlotModel(factory);
}

static void destroyModel(rfcommon::Plugin* model)
{
    delete model;
}

static RFPluginFactory factories[] = {
    {createModel, destroyModel, RFPluginType::REALTIME,
    {"Damage vs Time Plot",
    "misc",
    "TheComet",
    "TheComet#5387, @TheComet93",
    "Plots each player's damage over time"}},

    {nullptr}
};

static int start(uint32_t version)
{
    return 0;
}

static void stop()
{
}

DEFINE_PLUGIN(factories, start, stop)
