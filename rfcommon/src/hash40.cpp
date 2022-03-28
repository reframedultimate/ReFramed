#include "rfcommon/hash40.hpp"
#include "rfcommon/crc32.h"
#include <string.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterMotion hash40(const void* buf, uintptr_t len)
{
    return (uint64_t)crc32_buf(buf, len, 0) | ((uint64_t)len << 32);
}

// ----------------------------------------------------------------------------
FighterMotion hash40(const char* str)
{
    return hash40((const void*)str, strlen(str));
}

}
