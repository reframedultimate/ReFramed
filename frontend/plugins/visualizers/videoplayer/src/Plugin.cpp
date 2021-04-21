#include "videoplayer/PluginConfig.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/PluginFactory.hpp"
#include "uh/VisualizerPlugin.hpp"

#include <QWidget>

class VideoPlayer : public uh::VisualizerPlugin
                  , public QWidget
{
public:
    QWidget* getWidget() override { return this; }
};

class VideoPlayerFactory : public uh::PluginFactory
{
    uh::Plugin* create() override { return new VideoPlayer; }
    void destroy(uh::Plugin* plugin) override { delete plugin; }
    Type type() const override { return uh::PluginFactory::VISUALIZER; }
    const char* author() const override { return "TheComet"; }
    const char* contact() const override { return "alex.murray@gmx.ch"; }
    const char* description() const override { return ""; }
};

PLUGIN_API bool startPlugin(uh::PluginInterface* pi)
{
    return pi->registerFactory<VideoPlayerFactory>();
}

PLUGIN_API void stopPlugin(uh::PluginInterface* pi)
{
    pi->unregisterFactory<VideoPlayerFactory>();
}
