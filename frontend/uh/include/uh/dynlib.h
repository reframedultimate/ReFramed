#pragma once

#include "uh/config.hpp"

#ifdef __cplusplus
extern "C" {
#endif

struct uh_dynlib;

UH_PUBLIC_API struct uh_dynlib* uh_dynlib_open(const char* fileName);
UH_PUBLIC_API void uh_dynlib_close(struct uh_dynlib* dynlib);
UH_PUBLIC_API void* uh_dynlib_lookup_symbol_address(struct uh_dynlib* dynlib, const char* name);

#ifdef __cplusplus
}
#endif
