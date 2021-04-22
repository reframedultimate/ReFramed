#pragma once

#include "uh/config.hpp"
#include "uh/Reference.hpp"
#include "uh/PlayerState.hpp"
#include <cstdint>

namespace uh {

class Recording;
class Video;

class UH_PUBLIC_API DataPoint
{
public:
    DataPoint(const PlayerState& state, Recording* recording);

    friend void swap(DataPoint& a, DataPoint& b);

    Recording* recording() const { return recording_; }
    const PlayerState& state() const { return state_; }

    uint32_t combinedState() const;

private:
    Reference<Recording> recording_;
    PlayerState state_;
};

}
