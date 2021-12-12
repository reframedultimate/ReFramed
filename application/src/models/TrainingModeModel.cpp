#include "application/listeners/TrainingModeListener.hpp"
#include "application/models/TrainingModeModel.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/RealtimePlugin.hpp"
#include <QWidget>

namespace rfapp {

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
        rfcommon::Plugin* plugin = it.value();
        dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginStopped, it.key(), plugin);
        pluginManager_->destroyModel(plugin);
    }
}

// ----------------------------------------------------------------------------
QVector<QString> TrainingModeModel::availablePluginNames() const
{
    return pluginManager_->availableFactoryNames(RFPluginType::REALTIME | RFPluginType::STANDALONE);
}

// ----------------------------------------------------------------------------
QList<QString> TrainingModeModel::runningPluginNames() const
{
    return runningPlugins_.keys();
}

// ----------------------------------------------------------------------------
const RFPluginFactoryInfo* TrainingModeModel::getPluginInfo(const QString& pluginName) const
{
    return pluginManager_->getFactoryInfo(pluginName);
}

// ----------------------------------------------------------------------------
rfcommon::Plugin* TrainingModeModel::runningPlugin(const QString& name) const
{
    auto it = runningPlugins_.find(name);
    if (it == runningPlugins_.end())
        return nullptr;
    return it.value();
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::launchPlugin(const QString& pluginName)
{
    rfcommon::Plugin* plugin = pluginManager_->createModel(pluginName, RFPluginType::REALTIME | RFPluginType::STANDALONE);
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

    rfcommon::Plugin* plugin = it.value();
    dispatcher.dispatch(&TrainingModeListener::onTrainingModePluginStopped, pluginName, plugin);

    pluginManager_->destroyModel(plugin);
    runningPlugins_.erase(it);

    return true;
}

// ----------------------------------------------------------------------------
bool TrainingModeModel::isPluginRunning(const QString& pluginName) const
{
    return runningPlugins_.contains(pluginName);
}

}
