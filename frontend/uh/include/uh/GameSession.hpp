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

class UH_PUBLIC_API GameSession : virtual public Session
{
protected:
    GameSession(
            SmallVector<SmallString<15>, 8>&& playerNames,
            GameNumber gameNumber=1,
            SetNumber setNumber=1,
            SetFormat setFormat=SetFormat::FRIENDLIES
    );

public:
    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \param index Which player to get
     */
    const SmallString<15>& playerName(int playerIdx) const override;

    /*!
     * \brief Gets the current game number. Starts at 1 and counts upwards as
     * sets progress.
     */
    GameNumber gameNumber() const;

    /*!
     * \brief Gets the set number. Usually 1. This number is used to disambiguate
     * sets where the same two players play the same characters on the same day.
     */
    SetNumber setNumber() const;

    /*!
     * \brief Gets the format of the set, @see Recording::Format
     */
    SetFormat format() const;

protected:
    SmallVector<SmallString<15>, 8> playerNames_;
    GameNumber gameNumber_;
    SetNumber setNumber_;
    SetFormat format_;
};

}
