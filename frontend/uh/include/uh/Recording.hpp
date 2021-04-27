#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/SetFormat.hpp"
#include "uh/RefCounted.hpp"
#include "uh/String.hpp"

namespace uh {

class PlayerState;
class RecordingListener;

class UH_PUBLIC_API Recording : public RefCounted
{
public:
    Recording(MappingInfo&& mapping,
              SmallVector<FighterID, 8>&& playerFighterIDs,
              SmallVector<SmallString<15>, 8>&& playerTags,
              StageID stageID);

    bool saveAs(const String& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const { return mappingInfo_; }

    /*!
     * \brief Gets the number of players
     */
    int playerCount() const { return static_cast<int>(playerTags_.count()); }

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param index Which player to get
     */
    const SmallString<15>& playerTag(int index) const { return playerTags_[index]; }

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \param index Which player to get
     */
    const SmallString<15>& playerName(int index) const { return playerNames_[index]; }

    /*!
     * \brief Gets the fighter ID being used by the specified player.
     * \param index The player to get
     */
    FighterID playerFighterID(int index) const { return playerFighterIDs_[index]; }

    /*!
     * \brief Gets the stage ID being played on.
     */
    StageID stageID() const { return stageID_; }

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
     * \brief Returns the player index of the player that won.
     */
    int winner() const { return winner_; }

    /*!
     * \brief Gets the format of the set, @see Recording::Format
     */
    SetFormat format() const { return format_; }

    /*!
     * \brief Gets the absolute time of when the match started in
     * unix time (milli-seconds since Jan 1 1970). This marks the first
     * frame of gameplay, immediately after the 3-2-1-Go countdown completes.
     * May be slightly off by 1 second or so.
     */
    uint64_t timeStampStartedMs() const { return timeStarted_; }
    uint64_t timeStampEndedMs() const { return timeEnded_; }

    uint64_t gameLengthMs() const;

    int playerStateCount(int player) const { return static_cast<int>(playerStates_[player].count()); }

    const PlayerState& playerStateAt(int player, int idx) const { return playerStates_[player][idx]; }
    const PlayerState* playerStatesBegin(int player) const { return playerStates_[player].data(); }
    const PlayerState* playerStatesEnd(int player) const;

    const PlayerState& firstReceivedState() const { return playerStates_[0][0]; }

    ListenerDispatcher<RecordingListener> dispatcher;

protected:
    int findWinner() const;

protected:
    uint64_t timeStarted_;
    uint64_t timeEnded_;
    MappingInfo mappingInfo_;
    SmallVector<SmallString<15>, 8> playerTags_;
    SmallVector<SmallString<15>, 8> playerNames_;
    SmallVector<FighterID, 8> playerFighterIDs_;
    SmallVector<Vector<PlayerState>, 8> playerStates_;
    SetFormat format_;
    GameNumber gameNumber_ = 1;
    SetNumber setNumber_ = 1;
    int winner_ = 0;
    StageID stageID_;
};

}
