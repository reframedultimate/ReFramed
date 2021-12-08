#pragma once

#include "rfcommon/Plugin.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API StandalonePlugin : public Plugin
{
public:
    StandalonePlugin(RFPluginFactory* factory);
    virtual ~StandalonePlugin();
};

}
