#pragma once

#include "uh/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

UH_PUBLIC_API void crc32_init(void);
UH_PUBLIC_API uint32_t crc32_buf(const void* buf, uintptr_t len, uint32_t crc);
UH_PUBLIC_API uint32_t crc32_str(const char* str, uint32_t crc);

#ifdef __cplusplus
}
#endif
