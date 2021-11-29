#include "hit-analysis/PluginConfig.hpp"
#include "hit-analysis/HitAnalyzer.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/PluginType.hpp"

#include <QWidget>

static uh::Plugin* createModel(UHPluginFactory* factory)
{
    return new HitAnalyzer(factory);
}

static void destroyModel(uh::Plugin* model)
{
    delete model;
}

static QWidget* createView(uh::Plugin* model)
{
    HitAnalyzer* analyzer = dynamic_cast<HitAnalyzer*>(model);
    return analyzer;
}

static void destroyView(uh::Plugin* model, QWidget* view)
{
}

static UHPluginFactory factories[] = {
    {createModel, destroyModel, createView, destroyView, UHPluginType::ANALYZER,
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
