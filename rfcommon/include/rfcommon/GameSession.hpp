#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {

class FighterState;

class RFCOMMON_PUBLIC_API GameSession : virtual public Session
{
protected:
    GameSession(
            SmallVector<SmallString<15>, 2>&& playerNames,
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
     * \note If training mode, this will always be the same as the tag.
     * \param fighterIdx Which player to get
     */
    const SmallString<15>& name(int playerIdx) const override;

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
    SmallVector<SmallString<15>, 2> playerNames_;
    GameNumber gameNumber_;
    SetNumber setNumber_;
    SetFormat format_;
};

}
