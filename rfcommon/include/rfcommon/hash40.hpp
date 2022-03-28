#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"

namespace rfcommon {

RFCOMMON_PUBLIC_API FighterMotion hash40(const void* buf, uintptr_t len);
RFCOMMON_PUBLIC_API FighterMotion hash40(const char* str);

}
