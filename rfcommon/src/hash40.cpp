#include "rfcommon/hash40.hpp"
#include "rfcommon/crc32.h"
#include <string.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterMotion hash40(const void* buf, uintptr_t len)
{
    return FighterMotion::fromParts(
        len, 
        crc32_buf(buf, len, 0));
}

// ----------------------------------------------------------------------------
FighterMotion hash40(const char* str)
{
    return hash40(static_cast<const void*>(str), strlen(str));
}

}
