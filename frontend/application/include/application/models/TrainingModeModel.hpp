#pragma once

#include "uh/ListenerDispatcher.hpp"
#include <QString>
#include <QHash>

struct UHPluginFactoryInfo;
class QWidget;

namespace uh {
    class Plugin;
}

namespace uhapp {

class PluginManager;
class TrainingModeListener;

class TrainingModeModel
{
public:
    TrainingModeModel(PluginManager* pluginManager);
    ~TrainingModeModel();

    QVector<QString> availablePluginNames() const;
    QList<QString> runningPluginNames() const;
    const UHPluginFactoryInfo* getPluginInfo(const QString& pluginName) const;

    uh::Plugin* runningPlugin(const QString& name) const;
    bool launchPlugin(const QString& pluginName);
    bool stopPlugin(const QString& pluginName);
    bool isPluginRunning(const QString& pluginName) const;

    uh::ListenerDispatcher<TrainingModeListener> dispatcher;

private:
    PluginManager* pluginManager_;
    QHash<QString, uh::Plugin*> runningPlugins_;
};

}
