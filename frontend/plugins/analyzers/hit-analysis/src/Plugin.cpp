#include "hit-analysis/PluginConfig.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/PluginType.hpp"

#include <QWidget>

static uh::Plugin* createHitAnalyzer()
{
    //return new HitAnalyzer;;
    return nullptr;
}

static void destroy(uh::Plugin* plugin)
{
    //delete plugin;
}

static PluginFactory factories[] = {
    {createHitAnalyzer, destroy, uh::PluginType::ANALYZER, PLUGIN_VERSION,
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

PLUGIN_API PluginInterface plugin_interface = {
    start,
    stop,
    factories
};
