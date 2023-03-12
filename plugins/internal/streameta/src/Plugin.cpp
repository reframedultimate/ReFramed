#include "streameta/PluginConfig.hpp"
#include "streameta/StreametaPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

static rfcommon::Plugin* createStreametaPlugin(
    RFPluginFactory* factory,
    rfcommon::PluginContext* pluginCtx,
    rfcommon::Log* log,
    rfcommon::MotionLabels* labels)
{
    PROFILE(PluginGlobal, createStreametaPlugin);

    return new StreametaPlugin(factory);
}

static void destroyStreametaPlugin(rfcommon::Plugin* plugin)
{
    PROFILE(PluginGlobal, destroyStreametaPlugin);

    delete plugin;
}

static const RFPluginType streametaPluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME;

static RFPluginFactory factories[] = {
    {createStreametaPlugin, destroyStreametaPlugin, streametaPluginTypes,
    {"Streameta",
     "misc > misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "Integrate with the streameta platform"}},

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
