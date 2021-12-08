#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {

class ProtocolListener;
class RunningGameSession;
class RunningTrainingSession;

class RFCOMMON_PUBLIC_API RealtimePlugin : public Plugin
                                   , public ProtocolListener
{
public:
    RealtimePlugin(RFPluginFactory* factory);
    virtual ~RealtimePlugin();
};

}
