#include "uh/hash40.h"
#include "uh/crc32.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
uint64_t hash40_buf(const void* buf, uintptr_t len)
{
    return (uint64_t)crc32_buf(buf, len, 0) | ((uint64_t)len << 32);
}

/* ------------------------------------------------------------------------- */
uint64_t hash40_str(const char* str)
{
    return hash40_buf((const void*)str, strlen(str));
}
