#include "frame-data-list/PluginConfig.hpp"
#include "frame-data-list/models/FrameDataListModel.hpp"
#include "rfcommon/PluginInterface.hpp"

static rfcommon::Plugin* createModel(RFPluginFactory* factory)
{
    return new FrameDataListModel(factory);
}

static void destroyModel(rfcommon::Plugin* model)
{
    delete model;
}

static RFPluginFactory factories[] = {
    {createModel, destroyModel, RFPluginType::REALTIME,
    {"Frame Data List",
     "misc",
     "TheComet",
     "TheComet#5387, @TheComet93",
     "View the data of each frame"}},

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
