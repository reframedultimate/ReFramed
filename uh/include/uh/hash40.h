#pragma once

#include "uh/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

UH_PUBLIC_API uint64_t hash40_buf(const void* buf, uintptr_t len);
UH_PUBLIC_API uint64_t hash40_str(const char* str);

#ifdef __cplusplus
}
#endif
