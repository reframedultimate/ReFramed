#include "hit-analysis/PluginConfig.hpp"
#include "hit-analysis/HitAnalyzer.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/PluginType.hpp"

#include <QWidget>

static uh::Plugin* createHitAnalyzer()
{
    return new HitAnalyzer;
}

static void destroy(uh::Plugin* plugin)
{
    delete plugin;
}

static UHPluginFactory factories[] = {
    {createHitAnalyzer, destroy, UHPluginType::ANALYZER,
     "Hit Analysis", "TheComet", "alex.murray@gmx.ch", "Finds all instances where you got hit"},
    {NULL}
};

static int start(uint32_t version)
{
    return 0;
}

static void stop()
{
}

PLUGIN_API UHPluginInterface plugin_interface = {
    PLUGIN_VERSION,
    factories,
    start,
    stop
};
