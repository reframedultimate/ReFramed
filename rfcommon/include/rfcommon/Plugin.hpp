#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/ProtocolListener.hpp"

class QWidget;
struct RFPluginFactory;

namespace rfcommon {

class Session;

class RFCOMMON_PUBLIC_API Plugin
{
public:
    class UIInterface
    {
    public:
        virtual QWidget* createView() = 0;
        virtual void destroyView(QWidget* view) = 0;
    };

    class ReplayInterface
    {
    public:
        virtual void onGameSessionLoaded(rfcommon::Session* game) = 0;
        virtual void onGameSessionUnloaded(rfcommon::Session* game) = 0;
        virtual void onTrainingSessionLoaded(rfcommon::Session* training) = 0;
        virtual void onTrainingSessionUnloaded(rfcommon::Session* training) = 0;

        virtual void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) = 0;
        virtual void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) = 0;
    };

    class VisualizerInterface
    {
    public:
    };

    class RealtimeInterface : public ProtocolListener
    {
    public:
    };

    Plugin(RFPluginFactory* factory);
    virtual ~Plugin();

    virtual UIInterface* uiInterface() = 0;
    virtual ReplayInterface* replayInterface() = 0;
    virtual VisualizerInterface* visualizerInterface() = 0;
    virtual RealtimeInterface* realtimeInterface() = 0;

    const RFPluginFactory* factory() const;

private:
    const RFPluginFactory* const factory_;
};

}
