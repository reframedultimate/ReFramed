#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API HighresTimer
{
public:
    HighresTimer();
    ~HighresTimer();

    void reset();
    void start();
    void stop();
    uint64_t timePassedNS() const;

private:
    uint64_t startTime_;
    uint64_t stopTime_;
};

}
