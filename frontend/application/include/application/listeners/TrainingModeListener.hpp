#pragma once

#include <QString>

namespace uh {
    class RealtimePlugin;
}

namespace uhapp {

class TrainingModeListener
{
public:
    virtual void onTrainingModePluginLaunched(const QString& name, uh::RealtimePlugin* plugin) = 0;
    virtual void onTrainingModePluginStopped(const QString& name, uh::RealtimePlugin* plugin) = 0;
};

}
