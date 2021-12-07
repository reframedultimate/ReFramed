#pragma once

#include "uh/Plugin.hpp"
#include "uh/ProtocolListener.hpp"
#include "uh/ListenerDispatcher.hpp"

namespace uh {

class ProtocolListener;
class RunningGameSession;
class RunningTrainingSession;

class UH_PUBLIC_API RealtimePlugin : public Plugin
                                   , public ProtocolListener
{
public:
    RealtimePlugin(UHPluginFactory* factory);
    virtual ~RealtimePlugin();
};

}
