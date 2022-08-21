#pragma once

#include "rfcommon/RefCounted.hpp"
#include <cstdint>

namespace rfcommon {

class HighresTimer : public RefCounted
{
public:
    HighresTimer();

    void reset();
    void start();
    void stop();
    uint64_t getTimePassedInNanoSeconds() const;

private:
    uint64_t startTime_;
    uint64_t stopTime_;
};

}
