#pragma once

#include "rfcommon/config.hpp"
#include <cstdio>

namespace rfcommon {

RFCOMMON_PUBLIC_API wchar_t* utf8_to_utf16(const char* utf8, int utf8_bytes);
RFCOMMON_PUBLIC_API void utf16_free(wchar_t* utf16);

RFCOMMON_PUBLIC_API FILE* utf8_fopen_write(const char* utf8_filename, int utf8_filename_bytes);
RFCOMMON_PUBLIC_API int utf8_remove(const char* utf8_filename, int utf8_filename_bytes);

}
