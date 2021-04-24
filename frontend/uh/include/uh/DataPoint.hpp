#pragma once

#include "uh/config.hpp"
#include "uh/Reference.hpp"
#include "uh/PlayerState.hpp"
#include <cstdint>
#include <string>

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

    const std::string& playerName() const;

    uint32_t combinedState() const;

private:
    Reference<Recording> recording_;
    PlayerState state_;
    int playerIdx_;
};

}
