#pragma once

#include "rfcommon/config.hpp"
#include "nlohmann/json.hpp"

namespace rfcommon {

class MappingInfo;

RFCOMMON_PUBLIC_API uint32_t checksum(const MappingInfo& mappingInfo);
RFCOMMON_PUBLIC_API bool toUnfilteredJSON(nlohmann::json* json, const MappingInfo&);
RFCOMMON_PUBLIC_API bool toFilteredJSON(nlohmann::json* json, const MappingInfo&);
RFCOMMON_PUBLIC_API bool fromFilteredJSON(MappingInfo* mappingInfo, const nlohmann::json& json);

}
