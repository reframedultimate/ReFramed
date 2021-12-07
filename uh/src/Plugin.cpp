#include "uh/Plugin.hpp"

namespace uh {

// ----------------------------------------------------------------------------
Plugin::Plugin(UHPluginFactory* factory)
    : factory_(factory)
{
}

// ----------------------------------------------------------------------------
Plugin::~Plugin()
{
}

}
