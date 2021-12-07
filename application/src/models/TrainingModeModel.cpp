#include "application/listeners/TrainingModeListener.hpp"
#include "application/models/TrainingModeModel.hpp"
#include "application/models/PluginManager.hpp"
#include "uh/RealtimePlugin.hpp"
#include <QWidget>

namespace uhapp {

// ----------------------------------------------------------------------------
TrainingModeModel::TrainingModeModel(PluginManager* pluginManager)
    : pluginManager_(pluginManager)
{
}

// ----------------------------------------------------------------------------
TrainingModeModel::~TrainingModeModel()
{
    for (auto it = runningPlugins_.begin(); it != runningPlugins_.end(); ++it)
    {
        const QString& name = it.key();
        uh::Plugin* plugin = it.value();
        dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginStopped, it.key(), plugin);
        pluginManager_->destroyModel(name, plugin);
    }
}

// ----------------------------------------------------------------------------
QVector<QString> TrainingModeModel::availablePluginNames() const
{
    return pluginManager_->availableFactoryNames(UHPluginType::REALTIME | UHPluginType::STANDALONE);
}

// ----------------------------------------------------------------------------
QList<QString> TrainingModeModel::runningPluginNames() const
{
    return runningPlugins_.keys();
}

// ----------------------------------------------------------------------------
const UHPluginFactoryInfo* TrainingModeModel::getPluginInfo(const QString& pluginName) const
{
    return pluginManager_->getFactoryInfo(pluginName);
}

// ----------------------------------------------------------------------------
uh::Plugin* TrainingModeModel::runningPlugin(const QString& name) const
{
    auto it = runningPlugins_.find(name);
    if (it == runningPlugins_.end())
        return nullptr;
    return it.value();
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::launchPlugin(const QString& pluginName)
{
    uh::Plugin* plugin = pluginManager_->createModel(pluginName, UHPluginType::REALTIME | UHPluginType::STANDALONE);
    if (plugin == nullptr)
        return false;

    runningPlugins_.insert(pluginName, plugin);
    dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginLaunched, pluginName, plugin);

    return true;
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::stopPlugin(const QString& pluginName)
{
    auto it = runningPlugins_.find(pluginName);
    if (it == runningPlugins_.end())
        return false;

    const QString& name = it.key();
    uh::Plugin* plugin = it.value();
    dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginStopped, pluginName, plugin);

    pluginManager_->destroyModel(name, plugin);
    runningPlugins_.erase(it);

    return true;
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::isPluginRunning(const QString& pluginName) const
{
    return runningPlugins_.contains(pluginName);
}

}
