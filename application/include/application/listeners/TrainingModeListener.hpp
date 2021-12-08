#pragma once

#include <QString>

namespace rfcommon {
    class Plugin;
}

namespace rfapp {

class TrainingModeListener
{
public:
    virtual void onTrainingModePluginLaunched(const QString& name, rfcommon::Plugin* plugin) = 0;
    virtual void onTrainingModePluginStopped(const QString& name, rfcommon::Plugin* plugin) = 0;
};

}
