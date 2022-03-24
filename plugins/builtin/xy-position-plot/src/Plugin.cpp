#include "xy-positions-plot/PluginConfig.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createPlugin(RFPluginFactory* factory)
{
    return new XYPositionsPlotModel(factory);
}

static void destroyPlugin(rfcommon::Plugin* model)
{
    delete model;
}

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, RFPluginType::REALTIME, {
         "XY Positions Plot",
         "misc",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "Plots each player's X and Y positions"
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
