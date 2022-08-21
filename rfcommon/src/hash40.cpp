#include "rfcommon/hash40.hpp"
#include "rfcommon/crc32.h"
#include "rfcommon/Profiler.hpp"
#include <string.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterMotion hash40(const void* buf, uintptr_t len)
{
    PROFILE(hash40Global, hash40);

    return FighterMotion::fromParts(
        len, 
        crc32_buf(buf, len, 0));
}

// ----------------------------------------------------------------------------
FighterMotion hash40(const char* str)
{
    PROFILE(hash40Global, hash40);

    return hash40(static_cast<const void*>(str), strlen(str));
}

}
