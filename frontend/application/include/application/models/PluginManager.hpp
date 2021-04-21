#pragma once

#include "uh/PluginType.hpp"
#include <unordered_map>
#include <string>
#include <vector>

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
    bool loadPlugin(const std::string& fileName);
    bool unloadPlugin(const std::string& fileName);
    void unloadAllPlugins();

    std::vector<std::string> availableNames(uh::PluginType type) const;

    uh::TrainingModePlugin* createTrainingMode(const char* name);
    uh::AnalyzerPlugin* createAnalyzer(const char* name);
    uh::VisualizerPlugin* createVisualizer(const char* name);
    void destroy(uh::Plugin* plugin);

private:
    std::unordered_map<std::string, PluginFactory*> factories_;
    std::unordered_map<uh::Plugin*, PluginFactory*> activePlugins_;
    std::vector<uh_dynlib*> libraries_;
};

}
