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
    class RealtimePlugin;
    class VisualizerPlugin;
}

namespace uhapp {

class PluginManager
{
public:
    ~PluginManager();

    bool loadPlugin(const QString& fileName);

    QVector<QString> availableNames(UHPluginType type) const;
    const UHPluginInfo* getInfo(const QString& name) const;

    uh::RealtimePlugin* createRealtime(const QString& name);
    uh::AnalyzerPlugin* createAnalyzer(const QString& name);
    uh::VisualizerPlugin* createVisualizer(const QString& name);
    void destroy(uh::Plugin* plugin);

private:
    QHash<QString, UHPluginFactory*> factories_;
    QHash<uh::Plugin*, UHPluginFactory*> activePlugins_;
    QVector<uh_dynlib*> libraries_;
};

}
