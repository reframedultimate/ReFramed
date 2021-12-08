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

}
