#include "xy-positions-plot/PluginConfig.hpp"
#include "xy-positions-plot/XYPositionsPlotPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
{
    return new XYPositionsPlotPlugin(factory);
}

static void destroyPlugin(rfcommon::Plugin* model)
{
    delete model;
}

static const RFPluginType xyPositionsPlotPluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, xyPositionsPlotPluginTypes, {
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
