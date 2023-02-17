#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/PluginSharedData.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API PluginContext : public RefCounted
{
public:
    PluginContext();
    ~PluginContext();

    HashMap<String, PluginSharedData> sources;
    ListenerDispatcher<Plugin::SharedDataInterface> dispatcher;
};

}
