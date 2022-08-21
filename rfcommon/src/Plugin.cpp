#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
Plugin::Plugin(RFPluginFactory* factory)
    : factory_(factory)
{
}

// ----------------------------------------------------------------------------
Plugin::~Plugin()
{
}

// ----------------------------------------------------------------------------
const RFPluginFactory* Plugin::factory() const
{
    NOPROFILE();

    return factory_;
}

}
