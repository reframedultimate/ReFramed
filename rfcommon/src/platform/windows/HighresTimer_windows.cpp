#include "rfcommon/HighresTimer.hpp"
#include "rfcommon/Profiler.hpp"

#include <Windows.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
HighresTimer::HighresTimer() :
    startTime_(0),
    stopTime_(0)
{
}

// ----------------------------------------------------------------------------
void HighresTimer::reset()
{
    NOPROFILE();

    startTime_ = 0;
    stopTime_ = 0;
}

// ----------------------------------------------------------------------------
void HighresTimer::start()
{
    NOPROFILE();

    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    startTime_ = time.QuadPart;
}

// ----------------------------------------------------------------------------
void HighresTimer::stop()
{
    NOPROFILE();

    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    stopTime_ = time.QuadPart;
}

// ----------------------------------------------------------------------------
uint64_t HighresTimer::getTimePassedInNanoSeconds() const
{
    NOPROFILE();

    uint64_t elapsed;
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    elapsed = (stopTime_ - startTime_) * 1000000;
    elapsed /= frequency.QuadPart;

    return elapsed;
}

}
