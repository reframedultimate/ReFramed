#pragma once

#include <QString>

namespace uh {
    class Plugin;
}

namespace uhapp {

class TrainingModeListener
{
public:
    virtual void onTrainingModePluginLaunched(const QString& name, uh::Plugin* plugin) = 0;
    virtual void onTrainingModePluginStopped(const QString& name, uh::Plugin* plugin) = 0;
};

}
