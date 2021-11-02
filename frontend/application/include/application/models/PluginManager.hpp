#pragma once

#include "uh/PluginType.hpp"
#include <QString>
#include <QVector>
#include <QHash>

struct uh_dynlib;
struct UHPluginFactory;
struct UHPluginInterface;
struct UHPluginInfo;

namespace uh {
    class Plugin;
    class AnalyzerPlugin;
    class TrainingModePlugin;
    class VisualizerPlugin;
}

namespace uhapp {

class PluginManager
{
public:
    bool loadPlugin(const QString& fileName);
    bool unloadPlugin(const QString& fileName);
    void unloadAllPlugins();

    QVector<QString> availableNames(UHPluginType type) const;
    UHPluginInfo* getInfo(const QString& name) const;

    uh::TrainingModePlugin* createTrainingMode(const QString& name);
    uh::AnalyzerPlugin* createAnalyzer(const QString& name);
    uh::VisualizerPlugin* createVisualizer(const QString& name);
    void destroy(uh::Plugin* plugin);

private:
    QHash<QString, UHPluginFactory*> factories_;
    QHash<uh::Plugin*, UHPluginFactory*> activePlugins_;
    QVector<uh_dynlib*> libraries_;
};

}
