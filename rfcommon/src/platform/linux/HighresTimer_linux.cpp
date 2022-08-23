#include "rfcommon/HighresTimer.hpp"
#include "rfcommon/Profiler.hpp"

#include <ctime>

namespace rfcommon {

// ----------------------------------------------------------------------------
HighresTimer::HighresTimer() :
    startTime_(0),
    stopTime_(0)
{
    NOPROFILE();
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

    struct timespec t;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);

    startTime_ = t.tv_sec*1000000000 + t.tv_nsec;
}

// ----------------------------------------------------------------------------
void HighresTimer::stop()
{
    NOPROFILE();

    struct timespec t;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);

    stopTime_ = t.tv_sec*1000000000 + t.tv_nsec;
}

// ----------------------------------------------------------------------------
uint64_t HighresTimer::getTimePassedInNanoSeconds() const
{
    NOPROFILE();

    return stopTime_ - startTime_;
}

}

