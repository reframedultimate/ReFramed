#include "uh/models/ConfigAccessor.hpp"
#include "uh/models/Config.hpp"

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
