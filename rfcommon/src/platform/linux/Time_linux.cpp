#include "rfcommon/time.h"
#include "rfcommon/Profiler.hpp"

#include <iomanip>
#include <sstream>
#include <ctime>
#include <cerrno>
#include <cstring>

#include <sys/time.h>

/* ------------------------------------------------------------------------- */
uint64_t time_milli_seconds_since_epoch(void)
{
    PROFILE(time_linuxGlobal, time_milli_seconds_since_epoch);

    struct timeval tv;
#ifndef NDEBUG
    if (
#endif
        gettimeofday(&tv, NULL)
#ifndef NDEBUG
        != 0)
    {
        fprintf(stderr, "gettimeofday() failed: %s\n", strerror(errno));
    }
#else
        ;
#endif

    return ((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000);
}

/* ------------------------------------------------------------------------- */
uint64_t time_qt_to_milli_seconds_since_epoch(const char* str)
{
    PROFILE(time_linuxGlobal, time_qt_to_milli_seconds_since_epoch);

    std::tm t;
    std::stringstream ss(str);
    ss >> std::get_time(&t, "%a %b %d %H:%M:%S %Y %Z");
    return (uint64_t)mktime(&t) * 1000;
}
