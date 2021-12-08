#pragma once

#include "rfcommon/config.hpp"

#ifdef __cplusplus
extern "C" {
#endif

struct rfcommon_dynlib;

RFCOMMON_PUBLIC_API struct rfcommon_dynlib* rfcommon_dynlib_open(const char* fileName);
RFCOMMON_PUBLIC_API void rfcommon_dynlib_close(struct rfcommon_dynlib* dynlib);
RFCOMMON_PUBLIC_API void* rfcommon_dynlib_lookup_symbol_address(struct rfcommon_dynlib* dynlib, const char* name);

#ifdef __cplusplus
}
#endif
