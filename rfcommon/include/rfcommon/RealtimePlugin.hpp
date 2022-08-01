#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {

class Session;

class RFCOMMON_PUBLIC_API RealtimePlugin : public Plugin
                                         , public ProtocolListener
{
public:
    RealtimePlugin(RFPluginFactory* factory);
    virtual ~RealtimePlugin();

    virtual void onGameSessionLoaded(rfcommon::Session* game) = 0;
    virtual void onTrainingSessionLoaded(rfcommon::Session* training) = 0;
    virtual void onGameSessionUnloaded(rfcommon::Session* game) = 0;
    virtual void onTrainingSessionUnloaded(rfcommon::Session* training) = 0;

    /*virtual void onDataSetMatchReplayLoaded(rfcommon::SavedGameSession* session) = 0;
    virtual void onDataSetTrainingReplayLoaded(rfcommon::SavedTrainingSession* session) = 0;*/
};

}
