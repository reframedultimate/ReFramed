#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/SetFormat.hpp"
#include "uh/Session.hpp"
#include "uh/String.hpp"

namespace uh {

class PlayerState;

extern template class UH_TEMPLATE_API SmallVector<SmallString<15>, 8>;
extern template class UH_TEMPLATE_API SmallVector<FighterID, 8>;
extern template class UH_TEMPLATE_API SmallVector<Vector<PlayerState>, 8>;

class UH_PUBLIC_API GameSession : public Session
{
public:
    GameSession(MappingInfo&& mapping,
                SmallVector<FighterID, 8>&& playerFighterIDs,
                SmallVector<SmallString<15>, 8>&& playerTags,
                SmallVector<SmallString<15>, 8>&& playerNames,
                StageID stageID);

    /*!
     * \brief Gets the current game number. Starts at 1 and counts upwards as
     * sets progress.
     */
    GameNumber gameNumber() const { return gameNumber_; }

    /*!
     * \brief Gets the set number. Usually 1. This number is used to disambiguate
     * sets where the same two players play the same characters on the same day.
     */
    SetNumber setNumber() const { return setNumber_; }

    /*!
     * \brief Gets the format of the set, @see Recording::Format
     */
    SetFormat format() const { return format_; }

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \param index Which player to get
     */
    const SmallString<15>& playerName(int index) const { return playerNames_[index]; }

    /*!
     * \brief Gets the absolute time of when the session started in unix time
     * (milli-seconds since Jan 1 1970). May be slightly off by 1 second or so
     * depending on latency.
     *
     * In the case of a game session, this marks the first frame of gameplay,
     * immediately after the 3-2-1-Go countdown completes.
     *
     * In the case of a training session, this marks the first frame that was
     * received (may not be the first frame of training mode depending on when
     * the user connected).
     */
    uint64_t timeStampStartedMs() const;

protected:
    int findWinner() const;

protected:
    uh::SmallVector<uh::Vector<PlayerState>, 8> playerStates_;
    uh::SmallVector<uh::SmallString<15>, 8> playerNames_;
    GameNumber gameNumber_ = 1;
    SetNumber setNumber_ = 1;
    SetFormat format_ = SetFormat::FRIENDLIES;
};

}
