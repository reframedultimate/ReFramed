#pragma once

#include "uh/config.hpp"
#include "uh/String.hpp"
#include "uh/Reference.hpp"
#include "uh/PlayerState.hpp"

namespace uh {

class Recording;
class Video;

class UH_PUBLIC_API DataPoint
{
public:
    /*!
     * \brief Create a data point out of the player state.
     * \param state Player state to copy
     * \param recording The recording from which the state originated
     * \param player The player's index within that recording from which the
     * state originated.
     */
    DataPoint(const PlayerState& state, Recording* recording, int player);

    const PlayerState& state() const { return state_; }
    Recording* recording() const { return recording_; }
    int player() const { return playerIdx_;  }

    const SmallString<15>& playerName() const;

    uint32_t combinedState() const;

private:
    // Fix MSVC complaining about Vector::resize() requiring a default constructor. It
    // never gets called
    friend class Vector<DataPoint>;
    DataPoint() { assert(false); }

    Reference<Recording> recording_;
    PlayerState state_;
    int playerIdx_;
};

}
