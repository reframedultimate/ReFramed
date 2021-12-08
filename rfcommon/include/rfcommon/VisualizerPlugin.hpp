#pragma once

#include "rfcommon/Plugin.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API VisualizerPlugin : public Plugin
{
public:
    VisualizerPlugin(RFPluginFactory* factory);
    ~VisualizerPlugin();
};

}
