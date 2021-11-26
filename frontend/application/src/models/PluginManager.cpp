#include "application/models/PluginManager.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/AnalyzerPlugin.hpp"
#include "uh/VisualizerPlugin.hpp"
#include "uh/RealtimePlugin.hpp"
#include "uh/StandalonePlugin.hpp"
#include "uh/dynlib.h"
#include <cassert>
#include <QDebug>

namespace uhapp {

// ----------------------------------------------------------------------------
PluginManager::~PluginManager()
{
    for (auto it = activePlugins_.begin(); it != activePlugins_.end(); ++it)
    {
        uh::Plugin* plugin = it.key();
        UHPluginFactory* factory = it.value();
        factory->destroy(plugin);
    }

    for (const auto& dl : libraries_)
    {
        UHPluginInterface* i = static_cast<UHPluginInterface*>(uh_dynlib_lookup_symbol_address(dl, "plugin_interface"));
        assert(i != nullptr);
        i->stop();
        uh_dynlib_close(dl);
    }

    activePlugins_.clear();
    libraries_.clear();
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
QVector<QString> PluginManager::availableNames(UHPluginType type) const
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
const UHPluginInfo* PluginManager::getInfo(const QString &name) const
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    UHPluginFactory* factory = it.value();
    return &factory->info;
}

// ----------------------------------------------------------------------------
uh::AnalyzerPlugin* PluginManager::createAnalyzer(const QString& name)
{
    return static_cast<uh::AnalyzerPlugin*>(create(name, UHPluginType::ANALYZER));
}

// ----------------------------------------------------------------------------
uh::VisualizerPlugin* PluginManager::createVisualizer(const QString& name)
{
    return static_cast<uh::VisualizerPlugin*>(create(name, UHPluginType::VISUALIZER));
}

// ----------------------------------------------------------------------------
uh::RealtimePlugin* PluginManager::createRealtime(const QString& name)
{
    return static_cast<uh::RealtimePlugin*>(create(name, UHPluginType::REALTIME));
}

// ----------------------------------------------------------------------------
uh::StandalonePlugin* PluginManager::createStandalone(const QString& name)
{
    return static_cast<uh::StandalonePlugin*>(create(name, UHPluginType::STANDALONE));
}

// ----------------------------------------------------------------------------
uh::Plugin* PluginManager::create(const QString& name, UHPluginType type)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    UHPluginFactory* factory = it.value();
    if (!(factory->type & type))
        return nullptr;

    uh::Plugin* plugin = factory->create();
    if (plugin == nullptr)
        return nullptr;

    activePlugins_.insert(plugin, factory);
    return plugin;
}

// ----------------------------------------------------------------------------
void PluginManager::destroy(uh::Plugin* plugin)
{
    auto it = activePlugins_.find(plugin);
    assert(it != activePlugins_.end());
    UHPluginFactory* factory = it.value();

    factory->destroy(plugin);
    activePlugins_.erase(it);
}

}
