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
        uh::Plugin* plugin = it.value();
        dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginStopped, it.key(), plugin);
        pluginManager_->destroy(plugin);
    }
}

// ----------------------------------------------------------------------------
QVector<QString> TrainingModeModel::availablePluginNames() const
{
    return pluginManager_->availableNames(UHPluginType::REALTIME | UHPluginType::STANDALONE);
}

// ----------------------------------------------------------------------------
const UHPluginInfo* TrainingModeModel::getPluginInfo(const QString& pluginName) const
{
    return pluginManager_->getInfo(pluginName);
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::launchPlugin(const QString& pluginName)
{
    uh::Plugin* plugin = pluginManager_->create(pluginName, UHPluginType::REALTIME | UHPluginType::STANDALONE);
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

    uh::Plugin* plugin = it.value();
    dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginStopped, pluginName, plugin);

    pluginManager_->destroy(plugin);
    runningPlugins_.erase(it);

    return true;
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::isPluginRunning(const QString& pluginName) const
{
    return runningPlugins_.contains(pluginName);
}

}
