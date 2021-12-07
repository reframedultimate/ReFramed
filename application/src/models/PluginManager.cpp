#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "uh/RunningGameSession.hpp"
#include "uh/RunningGameSession.hpp"
#include "uh/PluginInterface.hpp"
#include "application/models/Protocol.hpp"
#include "uh/AnalyzerPlugin.hpp"
#include "uh/VisualizerPlugin.hpp"
#include "uh/RealtimePlugin.hpp"
#include "uh/StandalonePlugin.hpp"
#include "uh/dynlib.h"
#include <cassert>
#include <QDebug>
#include <QWidget>

namespace uhapp {

// ----------------------------------------------------------------------------
PluginManager::PluginManager(Protocol* protocol)
    : protocol_(protocol)
{
}

// ----------------------------------------------------------------------------
PluginManager::~PluginManager()
{
    for (const auto& dl : libraries_)
    {
        UHPluginInterface* i = static_cast<UHPluginInterface*>(uh_dynlib_lookup_symbol_address(dl, "plugin_interface"));
        assert(i != nullptr);
        i->stop();
        uh_dynlib_close(dl);
    }
}

// ----------------------------------------------------------------------------
bool PluginManager::loadPlugin(const QString& fileName)
{
    UHPluginInterface* i;
    qDebug() << "Loading " << fileName;
    uh_dynlib* dl = uh_dynlib_open(fileName.toStdString().c_str());
    if (dl == nullptr)
    {
        qDebug() << "Failed to load " << fileName;
        goto open_failed;
    }

    i = static_cast<UHPluginInterface*>(uh_dynlib_lookup_symbol_address(dl, "plugin_interface"));
    if (i == nullptr)
    {
        qDebug() << "Failed to lookup symbol 'plugin_interface' in " << fileName;
        goto init_plugin_failed;
    }
    if (i->start(0x000001) != 0)
    {
        qDebug() << "Call to start() failed in " << fileName;
        goto init_plugin_failed;
    }

    for (UHPluginFactory* factory = i->factories; factory->info.name != nullptr; ++factory)
    {
        if (factories_.contains(factory->info.name))
            goto duplicate_factory_name;
        factories_.insert(factory->info.name, factory);
        qDebug() << "  - Successfully registered plugin factory " << factory->info.name;
    }

    libraries_.push_back(dl);

    return true;

    duplicate_factory_name :
        for (UHPluginFactory* factory = i->factories; factory->info.name != nullptr; ++factory)
        {
            auto it = factories_.find(factory->info.name);
            if (it != factories_.end())
                factories_.erase(it);
        }
    init_plugin_failed :
        uh_dynlib_close(dl);
    open_failed :
        return false;
}

// ----------------------------------------------------------------------------
QVector<QString> PluginManager::availableFactoryNames(UHPluginType type) const
{
    QVector<QString> list;
    for (auto factory = factories_.begin(); factory != factories_.end(); ++factory)
    {
        if (!!(factory.value()->type & type))
            list.push_back(factory.value()->info.name);
    }
    return list;
}

// ----------------------------------------------------------------------------
const UHPluginFactoryInfo* PluginManager::getFactoryInfo(const QString &name) const
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    UHPluginFactory* factory = it.value();
    return &factory->info;
}

// ----------------------------------------------------------------------------
uh::AnalyzerPlugin* PluginManager::createAnalyzerModel(const QString& name)
{
    // XXX is dynamic_cast required here due to some of these having multiple inheritance?
    return static_cast<uh::AnalyzerPlugin*>(createModel(name, UHPluginType::ANALYZER));
}

// ----------------------------------------------------------------------------
uh::VisualizerPlugin* PluginManager::createVisualizerModel(const QString& name)
{
    return static_cast<uh::VisualizerPlugin*>(createModel(name, UHPluginType::VISUALIZER));
}

// ----------------------------------------------------------------------------
uh::RealtimePlugin* PluginManager::createRealtimeModel(const QString& name)
{
    uh::RealtimePlugin* plugin = static_cast<uh::RealtimePlugin*>(createModel(name, UHPluginType::REALTIME));
    protocol_->dispatcher.addListener(plugin);
    return plugin;
}

// ----------------------------------------------------------------------------
uh::StandalonePlugin* PluginManager::createStandaloneModel(const QString& name)
{
    return static_cast<uh::StandalonePlugin*>(createModel(name, UHPluginType::STANDALONE));
}

// ----------------------------------------------------------------------------
uh::Plugin* PluginManager::createModel(const QString& name, UHPluginType type)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    UHPluginFactory* factory = it.value();
    if (!(factory->type & type))
        return nullptr;

    uh::Plugin* model = factory->createModel(factory);

    uh::RealtimePlugin* realtime = dynamic_cast<uh::RealtimePlugin*>(model);
    if (realtime)
        protocol_->dispatcher.addListener(realtime);

    return model;
}

// ----------------------------------------------------------------------------
void PluginManager::destroyModel(const QString& name, uh::Plugin* model)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return;

    UHPluginFactory* factory = it.value();

    uh::RealtimePlugin* realtime = dynamic_cast<uh::RealtimePlugin*>(model);
    if (realtime)
        protocol_->dispatcher.removeListener(realtime);

    factory->destroyModel(model);
}

}
