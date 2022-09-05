#include "rfcommon/Profiler.hpp"
#include "application/models/ConfigAccessor.hpp"
#include "application/models/Config.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
ConfigAccessor::ConfigAccessor(Config* config)
    : config_(config)
{
}

// ----------------------------------------------------------------------------
nlohmann::json& ConfigAccessor::getConfig() const
{
    PROFILE(ConfigAccessor, getConfig);

    return config_->root;
}

// ----------------------------------------------------------------------------
void ConfigAccessor::saveConfig() const
{
    PROFILE(ConfigAccessor, saveConfig);

    config_->save();
}

}
