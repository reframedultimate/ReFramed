#include "rfcommon/Utf8.hpp"
#include "rfcommon/Profiler.hpp"

#include <cstdlib>
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
wchar_t* utf8_to_utf16(const char* utf8, int utf8_bytes)
{
    NOPROFILE();

    (void)utf8;
    (void)utf8_bytes;

    return nullptr;
}

// ----------------------------------------------------------------------------
void utf16_free(wchar_t* utf16)
{
    NOPROFILE();

    (void)utf16;

    assert(false);
}

// ----------------------------------------------------------------------------
FILE* utf8_fopen_wb(const char* utf8_filename, int utf8_filename_bytes)
{
    NOPROFILE();

    (void)utf8_filename_bytes;
    return fopen(utf8_filename, "wb");
}

// ----------------------------------------------------------------------------
int utf8_remove(const char* utf8_filename, int utf8_filename_bytes)
{
    NOPROFILE();

    (void)utf8_filename_bytes;
    return remove(utf8_filename);
}

}
