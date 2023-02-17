#include "overextension/PluginConfig.hpp"
#include "overextension/OverextensionPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

static rfcommon::Plugin* createOverextensionPlugin(
    RFPluginFactory* factory,
    rfcommon::PluginContext* pluginCtx,
    rfcommon::Log* log,
    rfcommon::UserMotionLabels* userLabels,
    rfcommon::Hash40Strings* hash40Strings)
{
    PROFILE(PluginGlobal, createOverextensionPlugin);

    return new OverextensionPlugin(factory, pluginCtx, userLabels);
}

static void destroyOverextensionPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyOverextensionPlugin);

    delete plugin;
}

static const RFPluginType overextensionPluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createOverextensionPlugin, destroyOverextensionPlugin, overextensionPluginTypes,
    {"Overextension",
     "misc > misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "Finds overextensions"}},

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
