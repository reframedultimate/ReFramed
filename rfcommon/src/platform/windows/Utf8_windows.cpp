#include "rfcommon/Utf8.hpp"
#include "rfcommon/Profiler.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdlib>

namespace rfcommon {
    
// ----------------------------------------------------------------------------
wchar_t* utf8_to_utf16(const char* utf8, int utf8_bytes)
{
    PROFILE(Utf8_windowsGlobal, utf8_to_utf16);

    int utf16_bytes = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_bytes, NULL, 0);
    if (utf16_bytes == 0)
        return nullptr;

    wchar_t* utf16 = (wchar_t*)malloc((sizeof(wchar_t) + 1) * utf16_bytes);
    if (utf16 == NULL)
        return nullptr;

    if (MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_bytes, utf16, utf16_bytes) == 0)
    {
        free(utf16);
        return nullptr;
    }

    utf16[utf16_bytes] = 0;

    return utf16;
}

// ----------------------------------------------------------------------------
void utf16_free(wchar_t* utf16)
{
    PROFILE(Utf8_windowsGlobal, utf16_free);

    free(utf16);
}

// ----------------------------------------------------------------------------
FILE* utf8_fopen_write(const char* utf8_filename, int utf8_filename_bytes)
{
    PROFILE(Utf8_windowsGlobal, utf8_fopen_write);

    wchar_t* utf16_filename = utf8_to_utf16(utf8_filename, utf8_filename_bytes);
    if (utf16_filename == nullptr)
        return nullptr;

    FILE* fp = _wfopen(utf16_filename, L"wb");
    free(utf16_filename);

    return fp;
}

// ----------------------------------------------------------------------------
int utf8_remove(const char* utf8_filename, int utf8_filename_bytes)
{
    PROFILE(Utf8_windowsGlobal, utf8_remove);

    wchar_t* utf16_filename = utf8_to_utf16(utf8_filename, utf8_filename_bytes);
    if (utf16_filename == nullptr)
        return -1;

    int result = _wremove(utf16_filename);
    free(utf16_filename);
    return result;
}

}
