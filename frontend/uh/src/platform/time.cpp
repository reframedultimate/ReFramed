#include "uh/time.h"
#include <iomanip>
#include <sstream>

#if defined(WIN32)
#   include <windows.h>
#else
#   include <time.h>
#   include <sys/time.h>
#	include <errno.h>
#endif

/* ------------------------------------------------------------------------- */
uint64_t time_milli_seconds_since_epoch(void)
{
#if defined(WIN32)
    FILETIME ft;
    uint64_t ns100;
    GetSystemTimeAsFileTime(&ft);
    /* Number of 100ns intervals since January 1, 1601 (UTC) */
    ns100 = ((uint64_t)ft.dwHighDateTime <<32) + (uint64_t)ft.dwLowDateTime;
    /* Jan 1 1601 -> Jan 1 1970, convert to ms */
    return (ns100 - 116444736000000000LL) / 10000;
#else
    struct timeval tv;
#	ifdef DEBUG
    if (
#	endif
        gettimeofday(&tv, NULL)
#	ifdef DEBUG
        != 0)
    {
        fprintf(stderr, "gettimeofday() failed: %s\n", strerror(errno))
    }
#	else
        ;
#	endif

    return ((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000);
#endif
}

/* ------------------------------------------------------------------------- */
uint64_t time_qt_to_milli_seconds_since_epoch(const char* str)
{
    std::tm t;
    std::stringstream ss(str);
    ss >> std::get_time(&t, "%a %b %d %H:%M:%S %Y %Z");
    return (uint64_t)mktime(&t) * 1000;
}
