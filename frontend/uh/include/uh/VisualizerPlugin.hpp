#pragma once

#include "uh/Plugin.hpp"

namespace uh {

class UH_PUBLIC_API VisualizerPlugin : public Plugin
{
public:
    VisualizerPlugin(UHPluginFactory* factory);
    ~VisualizerPlugin();
};

}
