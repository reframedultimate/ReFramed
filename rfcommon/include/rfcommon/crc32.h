#pragma once

#include "rfcommon/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

RFCOMMON_PUBLIC_API void crc32_init(void);
RFCOMMON_PUBLIC_API uint32_t crc32_buf(const void* buf, uintptr_t len, uint32_t crc);
RFCOMMON_PUBLIC_API uint32_t crc32_str(const char* str, uint32_t crc);

#ifdef __cplusplus
}
#endif
