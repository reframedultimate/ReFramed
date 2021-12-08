#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include <QString>
#include <QHash>

struct RFPluginFactoryInfo;
class QWidget;

namespace rfcommon {
    class Plugin;
}

namespace rfapp {

class PluginManager;
class TrainingModeListener;

class TrainingModeModel
{
public:
    TrainingModeModel(PluginManager* pluginManager);
    ~TrainingModeModel();

    QVector<QString> availablePluginNames() const;
    QList<QString> runningPluginNames() const;
    const RFPluginFactoryInfo* getPluginInfo(const QString& pluginName) const;

    rfcommon::Plugin* runningPlugin(const QString& name) const;
    bool launchPlugin(const QString& pluginName);
    bool stopPlugin(const QString& pluginName);
    bool isPluginRunning(const QString& pluginName) const;

    rfcommon::ListenerDispatcher<TrainingModeListener> dispatcher;

private:
    PluginManager* pluginManager_;
    QHash<QString, rfcommon::Plugin*> runningPlugins_;
};

}
