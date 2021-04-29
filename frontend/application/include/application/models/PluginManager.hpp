#pragma once

#include "uh/PluginType.hpp"
#include "uh/String.hpp"
#include "uh/HashMap.hpp"
#include <QString>

struct uh_dynlib;
struct PluginFactory;
struct PluginInterface;

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

    uh::Vector<uh::String> availableNames(uh::PluginType type) const;

    uh::TrainingModePlugin* createTrainingMode(const uh::String& name);
    uh::AnalyzerPlugin* createAnalyzer(const uh::String& name);
    uh::VisualizerPlugin* createVisualizer(const uh::String& name);
    void destroy(uh::Plugin* plugin);

private:
    uh::HashMap<uh::String, PluginFactory*> factories_;
    uh::HashMap<uh::Plugin*, PluginFactory*> activePlugins_;
    uh::Vector<uh_dynlib*> libraries_;
};

}
