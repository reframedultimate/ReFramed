#include "rfcommon/Plugin.hpp"

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
RFPluginFactory* Plugin::factory() const
{
    return factory_;
}

}
