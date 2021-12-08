#include "hit-analysis/PluginConfig.hpp"
#include "hit-analysis/HitAnalyzer.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/PluginType.hpp"

#include <QWidget>

static rfcommon::Plugin* createModel(RFPluginFactory* factory)
{
    return new HitAnalyzer(factory);
}

static void destroyModel(rfcommon::Plugin* model)
{
    delete model;
}

static QWidget* createView(rfcommon::Plugin* model)
{
    HitAnalyzer* analyzer = dynamic_cast<HitAnalyzer*>(model);
    return analyzer;
}

static void destroyView(rfcommon::Plugin* model, QWidget* view)
{
}

static RFPluginFactory factories[] = {
    {createModel, destroyModel, createView, destroyView, RFPluginType::ANALYZER,
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

DEFINE_PLUGIN(factories, start, stop)
