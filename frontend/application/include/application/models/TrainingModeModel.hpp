#pragma once

#include "uh/ListenerDispatcher.hpp"
#include <QString>
#include <QHash>

struct UHPluginInfo;
class QWidget;

namespace uh {
    class RealtimePlugin;
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
    const UHPluginInfo* getPluginInfo(const QString& pluginName) const;

    bool launchPlugin(const QString& pluginName);
    bool stopPlugin(const QString& pluginName);
    bool isPluginRunning(const QString& pluginName) const;

    uh::ListenerDispatcher<TrainingModeListener> dispatcher;

private:
    PluginManager* pluginManager_;
    QHash<QString, uh::RealtimePlugin*> runningPlugins_;
};

}
