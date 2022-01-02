#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {

class SavedGameSession;

class RFCOMMON_PUBLIC_API RealtimePlugin : public Plugin
                                         , public ProtocolListener
{
public:
    RealtimePlugin(RFPluginFactory* factory);
    virtual ~RealtimePlugin();

    virtual void setSavedGameSession(rfcommon::SavedGameSession* session) = 0;
    virtual void clearSavedGameSession(rfcommon::SavedGameSession* session) = 0;

    /*virtual void onDataSetMatchReplayLoaded(rfcommon::SavedGameSession* session) = 0;
    virtual void onDataSetTrainingReplayLoaded(rfcommon::SavedTrainingSession* session) = 0;*/
};

}
