#include "application/models/PluginManager.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/AnalyzerPlugin.hpp"
#include "uh/TrainingModePlugin.hpp"
#include "uh/VisualizerPlugin.hpp"
#include "uh/dynlib.h"
#include <cassert>

namespace uhapp {

// ----------------------------------------------------------------------------
bool PluginManager::loadPlugin(const QString& fileName)
{
    PluginInterface* i;
    uh_dynlib* dl = uh_dynlib_open(fileName.toStdString().c_str());
    if (dl == nullptr)
        goto open_failed;

    i = static_cast<PluginInterface*>(uh_dynlib_lookup_symbol_address(dl, "plugin_interface"));
    if (i == nullptr)
        goto init_plugin_failed;
    if (i->start(0x000001) != 0)
        goto init_plugin_failed;

    for (PluginFactory* factory = i->factories; factory->name != nullptr; ++factory)
    {
        if (factories_.contains(factory->name))
            goto duplicate_factory_name;
        factories_.insert(factory->name, factory);
    }

    libraries_.push_back(dl);

    return true;

    duplicate_factory_name :
        for (PluginFactory* factory = i->factories; factory->name != nullptr; ++factory)
        {
            auto it = factories_.find(factory->name);
            if (it != factories_.end())
                factories_.erase(it);
        }
    init_plugin_failed :
        uh_dynlib_close(dl);
    open_failed :
        return false;
}

// ----------------------------------------------------------------------------
bool PluginManager::unloadPlugin(const QString& fileName)
{
    return false;
}

// ----------------------------------------------------------------------------
void PluginManager::unloadAllPlugins()
{
    for (auto it = activePlugins_.begin(); it != activePlugins_.end(); ++it)
    {
        uh::Plugin* plugin = it.key();
        PluginFactory* factory = it.value();
        factory->destroy(plugin);
    }

    for (const auto& dl : libraries_)
    {
        PluginInterface* i = static_cast<PluginInterface*>(uh_dynlib_lookup_symbol_address(dl, "plugin_interface"));
        assert(i != nullptr);
        i->stop();
        uh_dynlib_close(dl);
    }

    activePlugins_.clear();
    libraries_.clear();
}

// ----------------------------------------------------------------------------
QVector<QString> PluginManager::availableNames(uh::PluginType type) const
{
    QVector<QString> list;
    for (auto factory = factories_.begin(); factory != factories_.end(); ++factory)
    {
        if (factory.value()->type == type)
            list.push_back(factory.value()->name);
    }
    return list;
}

// ----------------------------------------------------------------------------
uh::TrainingModePlugin* PluginManager::createTrainingMode(const QString& name)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    PluginFactory* factory = it.value();
    if (factory->type != uh::PluginType::TRAINING_MODE)
        return nullptr;

    uh::TrainingModePlugin* plugin = static_cast<uh::TrainingModePlugin*>(factory->create());
    if (plugin == nullptr)
        return nullptr;

    activePlugins_.insert(plugin, factory);
    return plugin;
}

// ----------------------------------------------------------------------------
uh::AnalyzerPlugin* PluginManager::createAnalyzer(const QString& name)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    PluginFactory* factory = it.value();
    if (factory->type != uh::PluginType::ANALYZER)
        return nullptr;

    uh::AnalyzerPlugin* plugin = static_cast<uh::AnalyzerPlugin*>(factory->create());
    if (plugin == nullptr)
        return nullptr;

    activePlugins_.insert(plugin, factory);
    return plugin;
}

// ----------------------------------------------------------------------------
uh::VisualizerPlugin* PluginManager::createVisualizer(const QString& name)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    PluginFactory* factory = it.value();
    if (factory->type != uh::PluginType::VISUALIZER)
        return nullptr;

    uh::VisualizerPlugin* plugin = static_cast<uh::VisualizerPlugin*>(factory->create());
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
    PluginFactory* factory = it.value();

    factory->destroy(plugin);
    activePlugins_.erase(it);
}

}
