#include "application/models/ConfigAccessor.hpp"
#include "application/models/Config.hpp"

namespace uh {

// ----------------------------------------------------------------------------
ConfigAccessor::ConfigAccessor(Config* config)
    : config_(config)
{
}

// ----------------------------------------------------------------------------
QJsonObject& ConfigAccessor::getConfig() const
{
    return config_->root;
}

// ----------------------------------------------------------------------------
void ConfigAccessor::saveConfig() const
{
    config_->save();
}

}
