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
nlohmann::json& ConfigAccessor::configRoot() const
{
    PROFILE(ConfigAccessor, getConfig);

    return config_->root;
}

// ----------------------------------------------------------------------------
Config* ConfigAccessor::config() const
{
    return config_;
}

// ----------------------------------------------------------------------------
void ConfigAccessor::saveConfig() const
{
    PROFILE(ConfigAccessor, saveConfig);

    config_->save();
}

}
