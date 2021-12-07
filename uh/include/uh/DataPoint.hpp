#pragma once

#include "uh/config.hpp"
#include "uh/String.hpp"
#include "uh/Reference.hpp"
#include "uh/PlayerState.hpp"

namespace uh {

class SavedGameSession;
class Video;

extern template class UH_TEMPLATE_API Reference<SavedGameSession>;

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
    DataPoint(const PlayerState& state, SavedGameSession* session, int player);

    const PlayerState& state() const { return state_; }
    SavedGameSession* session() const { return session_; }
    int player() const { return playerIdx_;  }

    const SmallString<15>& playerName() const;

    uint32_t combinedState() const;

private:
    // Fix MSVC complaining about Vector::resize() requiring a default constructor. It
    // never gets called
    friend class Vector<DataPoint>;
    DataPoint() { assert(false); }

    Reference<SavedGameSession> session_;
    PlayerState state_;
    int playerIdx_;
};

}
