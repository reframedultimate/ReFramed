#include "application/models/PluginManager.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/VisualizerPlugin.hpp"
#include "uh/dynlib.h"
#include <cassert>

namespace uhapp {

// ----------------------------------------------------------------------------
bool PluginManager::loadPlugin(const std::string& fileName)
{
    PluginInterface* i;
    uh_dynlib* dl = uh_dynlib_open(fileName.c_str());
    if (dl == nullptr)
        goto open_failed;

    i = static_cast<PluginInterface*>(uh_dynlib_lookup_symbol_address(dl, "plugin_interface"));
    if (i == nullptr)
        goto init_plugin_failed;
    if (i->start(0x000001) != 0)
        goto init_plugin_failed;

    for (PluginFactory* factory = i->factories; factory->name != nullptr; ++factory)
    {
        auto result = factories_.emplace(factory->name, factory);
        if (result.second == false)
            goto duplicate_factory_name;
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
bool PluginManager::unloadPlugin(const std::string& fileName)
{
    return false;
}

// ----------------------------------------------------------------------------
void PluginManager::unloadAllPlugins()
{
    for (const auto& [plugin, factory] : activePlugins_)
    {
        plugin->giveWidget();
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
std::vector<std::string> PluginManager::availableNames(uh::PluginType type) const
{
    std::vector<std::string> list;
    for (const auto& [name, factory] : factories_)
    {
        if (factory->type == type)
            list.emplace_back(factory->name);
    }
    return list;
}

// ----------------------------------------------------------------------------
uh::TrainingModePlugin* PluginManager::createTrainingMode(const char* name)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    PluginFactory* factory = it->second;
    if (factory->type != uh::PluginType::TRAINING_MODE)
        return nullptr;

    uh::TrainingModePlugin* plugin = static_cast<uh::TrainingModePlugin*>(factory->create());
    if (plugin == nullptr)
        return nullptr;

    activePlugins_.emplace(plugin, factory);
    return plugin;
}

// ----------------------------------------------------------------------------
uh::AnalyzerPlugin* PluginManager::createAnalyzer(const char* name)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    PluginFactory* factory = it->second;
    if (factory->type != uh::PluginType::ANALYZER)
        return nullptr;

    uh::AnalyzerPlugin* plugin = static_cast<uh::AnalyzerPlugin*>(factory->create());
    if (plugin == nullptr)
        return nullptr;

    activePlugins_.emplace(plugin, factory);
    return plugin;
}

// ----------------------------------------------------------------------------
uh::VisualizerPlugin* PluginManager::createVisualizer(const char* name)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    PluginFactory* factory = it->second;
    if (factory->type != uh::PluginType::VISUALIZER)
        return nullptr;

    uh::VisualizerPlugin* plugin = static_cast<uh::VisualizerPlugin*>(factory->create());
    if (plugin == nullptr)
        return nullptr;

    activePlugins_.emplace(plugin, factory);
    return plugin;
}

// ----------------------------------------------------------------------------
void PluginManager::destroy(uh::Plugin* plugin)
{
    auto it = activePlugins_.find(plugin);
    assert(it != activePlugins_.end());
    PluginFactory* factory = it->second;

    plugin->giveWidget();
    factory->destroy(plugin);
}

}
