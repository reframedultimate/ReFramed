#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/VisualizerData.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API VisualizerContext : public RefCounted
{
public:
    VisualizerContext();
    ~VisualizerContext();

    HashMap<String, VisualizerData> sources;
    ListenerDispatcher<Plugin::VisualizerInterface> dispatcher;
};

}
