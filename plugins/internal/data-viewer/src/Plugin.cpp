#include "data-viewer/PluginConfig.hpp"
#include "data-viewer/DataViewerPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"

static rfcommon::Plugin* createPlugin(
    RFPluginFactory* factory,
    rfcommon::PluginContext* pluginCtx,
    rfcommon::Log* log,
    rfcommon::UserMotionLabels* userLabels,
    rfcommon::Hash40Strings* hash40Strings)
{
    PROFILE(PluginGlobal, createPlugin);

    return new DataViewerPlugin(factory, userLabels, hash40Strings);
}

static void destroyPlugin(rfcommon::Plugin* model)
{
    PROFILE(PluginGlobal, destroyPlugin);

    delete model;
}

static const RFPluginType dataViewerPluginTypes =
    RFPluginType::UI |
    RFPluginType::REALTIME |
    RFPluginType::REPLAY;

static RFPluginFactory factories[] = {
    {createPlugin, destroyPlugin, dataViewerPluginTypes, {
         "Data Viewer",
         "misc",
         "TheComet",
         "TheComet#5387, @TheComet93",
         "View raw data in replay files such as metadata, mapping info, and frame data"
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
