#pragma once

#include "rfcommon/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

RFCOMMON_PUBLIC_API uint64_t hash40_buf(const void* buf, uintptr_t len);
RFCOMMON_PUBLIC_API uint64_t hash40_str(const char* str);

#ifdef __cplusplus
}
#endif
