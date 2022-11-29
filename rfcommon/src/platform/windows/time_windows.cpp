#include "rfcommon/time.h"
#include "rfcommon/Profiler.hpp"
#include <iomanip>
#include <sstream>

#include <windows.h>

/* ------------------------------------------------------------------------- */
uint64_t time_milli_seconds_since_epoch(void)
{
    NOPROFILE();

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return time_win32_filetime_to_milli_seconds_since_epoch(
            ((uint64_t)ft.dwHighDateTime <<32) + (uint64_t)ft.dwLowDateTime);
}

/* ------------------------------------------------------------------------- */
uint64_t time_qt_to_milli_seconds_since_epoch(const char* str)
{
    NOPROFILE();

    std::tm t;
    std::stringstream ss(str);
    ss >> std::get_time(&t, "%a %b %d %H:%M:%S %Y %Z");
    return (uint64_t)mktime(&t) * 1000;
}

/* ------------------------------------------------------------------------- */
uint64_t time_win32_filetime_to_milli_seconds_since_epoch(uint64_t quadpart)
{
    NOPROFILE();

    /* quadpart represents the number of 100ns intervals since January 1, 1601 (UTC) */
    /* Jan 1 1601 -> Jan 1 1970, convert to ms */

    return (quadpart - 116444736000000000LL) / 10000;
}

